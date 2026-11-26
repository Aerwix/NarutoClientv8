/*
 * Copyright (c) 2010-2025 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "discord.h"

#if ENABLE_DISCORD_RPC == 1
#include <framework/core/eventdispatcher.h>
#include <framework/core/logger.h>
#include <framework/stdext/format.h>
#include <cctype>
#include <chrono>
#include <cstring>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

Discord g_discord;

namespace {

enum : uint32_t { OP_HANDSHAKE = 0, OP_FRAME = 1, OP_CLOSE = 2, OP_PING = 3, OP_PONG = 4 };

constexpr int POLL_INTERVAL = 1000;       // ms: read incoming frames / check changes
constexpr int64_t RECONNECT_DELAY = 5;    // s between connection attempts

const int64_t START_TIME = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();

int64_t nowSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::string jsonEscape(const std::string& in)
{
    std::string out;
    out.reserve(in.size() + 8);
    for (unsigned char c : in) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) out += stdext::format("\\u%04x", (int)c);
                else out += (char)c;
        }
    }
    return out;
}

std::string jsonStr(const std::string& s) { return "\"" + jsonEscape(s) + "\""; }

// Cuts to at most maxBytes without splitting a multi-byte UTF-8 character (•, emojis...).
std::string utf8Truncate(const std::string& s, size_t maxBytes)
{
    if (s.size() <= maxBytes)
        return s;
    size_t cut = maxBytes;
    // UTF-8 continuation bytes look like 10xxxxxx; back up to the start of that character
    while (cut > 0 && (static_cast<unsigned char>(s[cut]) & 0xC0) == 0x80)
        --cut;
    return s.substr(0, cut);
}

// crude extraction of a string field from a Discord response, good enough for logging
std::string jsonField(const std::string& json, const std::string& key)
{
    const std::string needle = "\"" + key + "\":\"";
    const size_t pos = json.find(needle);
    if (pos == std::string::npos) return {};
    const size_t start = pos + needle.size();
    const size_t end = json.find('"', start);
    return end == std::string::npos ? std::string{} : json.substr(start, end - start);
}

std::string urlEncode(const std::string& in)
{
    std::string out;
    for (unsigned char c : in) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            out += (char)c;
        else if (c == ' ')
            out += '+';
        else
            out += stdext::format("%%%02X", (int)c);
    }
    return out;
}

// Adds a button. "{name}" in the url is replaced by the url-encoded character name;
// if the url needs a name and there is none (not logged in), the button is skipped.
void appendButton(std::string& out, const char* label, const char* urlTemplate, const std::string& name)
{
    if (!label || !urlTemplate || !*label || !*urlTemplate) return;

    std::string url = urlTemplate;
    const size_t pos = url.find("{name}");
    if (pos != std::string::npos) {
        if (name.empty()) return;
        url.replace(pos, 6, urlEncode(name));
    }

    if (!out.empty()) out += ",";
    out += "{\"label\":" + jsonStr(label) + ",\"url\":" + jsonStr(url) + "}";
}

} // namespace

void Discord::init(std::function<bool()> canUpdate, std::function<void(std::string&)> onUpdate,
                   std::function<void(std::string&)> onState, std::function<std::string()> getName)
{
    if (m_initialized || std::strlen(RPC_API_KEY) == 0)
        return;

    m_canUpdate = std::move(canUpdate);
    m_onUpdate = std::move(onUpdate);
    m_onState = std::move(onState);
    m_getName = std::move(getName);
    m_initialized = true;
    g_logger.info("[Discord] Rich Presence iniciado");
    poll();
}

void Discord::terminate()
{
    if (!m_initialized)
        return;
    m_initialized = false;
    if (m_pipe) {
        // clear activity before leaving
        writeFrame(OP_FRAME, stdext::format("{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%d},\"nonce\":\"%d\"}",
                                            (int)GetCurrentProcessId(), (int)++m_nonce));
    }
    disconnect();
}

void Discord::poll()
{
    if (!m_initialized)
        return;

    if (!m_pipe && nowSeconds() - m_lastConnectAttempt >= RECONNECT_DELAY) {
        m_lastConnectAttempt = nowSeconds();
        if (connect()) {
            m_lastDetails.clear(); // force resend on new connection
        }
    }

    if (m_pipe)
        readFrames();

    if (m_pipe) {
        std::string details;
        std::string state;
        std::string name;
        if (m_canUpdate && m_canUpdate()) {
            if (m_onUpdate) m_onUpdate(details);
            if (m_onState) m_onState(state);
            if (m_getName) name = m_getName();
        }
        if (details.empty())
            details = OFFLINE_RPC_TEXT;
        if (state.size() < 2) // Discord rejects state shorter than 2 chars
            state = STATE_RPC_TEXT;

        std::string buttons;
        appendButton(buttons, RPC_BUTTON1_LABEL, RPC_BUTTON1_URL, name);
        appendButton(buttons, RPC_BUTTON2_LABEL, RPC_BUTTON2_URL, name);

        const bool changed = details != m_lastDetails || state != m_lastState || buttons != m_lastButtons;
        const bool stale = nowSeconds() - m_lastSend >= RPC_UPDATE_INTERVAL / 1000;
        if (changed || stale)
            sendPresence(details, state, buttons);
    }

    g_dispatcher.scheduleEvent([this] { poll(); }, POLL_INTERVAL);
}

bool Discord::connect()
{
    for (int i = 0; i < 10; ++i) {
        const std::string name = stdext::format("\\\\.\\pipe\\discord-ipc-%d", i);
        HANDLE pipe = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (pipe == INVALID_HANDLE_VALUE)
            continue;

        m_pipe = pipe;
        const std::string handshake = std::string("{\"v\":1,\"client_id\":") + jsonStr(RPC_API_KEY) + "}";
        if (!writeFrame(OP_HANDSHAKE, handshake)) {
            disconnect();
            continue;
        }
        g_logger.info(stdext::format("[Discord] Conectado a discord-ipc-%d", i));
        return true;
    }
    return false;
}

void Discord::disconnect()
{
    if (m_pipe) {
        CloseHandle((HANDLE)m_pipe);
        m_pipe = nullptr;
    }
}

bool Discord::writeFrame(uint32_t opcode, const std::string& payload)
{
    if (!m_pipe)
        return false;

    std::vector<char> buffer(8 + payload.size());
    const uint32_t length = (uint32_t)payload.size();
    std::memcpy(buffer.data(), &opcode, 4);
    std::memcpy(buffer.data() + 4, &length, 4);
    std::memcpy(buffer.data() + 8, payload.data(), payload.size());

    DWORD written = 0;
    if (!WriteFile((HANDLE)m_pipe, buffer.data(), (DWORD)buffer.size(), &written, nullptr) || written != buffer.size()) {
        g_logger.warning("[Discord] Conexion perdida, reintentando...");
        disconnect();
        return false;
    }
    return true;
}

void Discord::readFrames()
{
    while (m_pipe) {
        DWORD available = 0;
        if (!PeekNamedPipe((HANDLE)m_pipe, nullptr, 0, nullptr, &available, nullptr)) {
            g_logger.warning("[Discord] Conexion perdida, reintentando...");
            disconnect();
            return;
        }
        if (available < 8)
            return;

        uint32_t header[2] = { 0, 0 };
        DWORD read = 0;
        if (!PeekNamedPipe((HANDLE)m_pipe, header, 8, &read, nullptr, nullptr) || read < 8)
            return;
        if (available < 8 + header[1])
            return; // wait for the full frame

        std::vector<char> data(8 + header[1]);
        if (!ReadFile((HANDLE)m_pipe, data.data(), (DWORD)data.size(), &read, nullptr) || read != data.size()) {
            disconnect();
            return;
        }
        const std::string payload(data.data() + 8, header[1]);

        switch (header[0]) {
            case OP_FRAME:
                if (payload.find("\"evt\":\"READY\"") != std::string::npos)
                    g_logger.info(stdext::format("[Discord] Listo, usuario: %s", jsonField(payload, "username")));
                else if (payload.find("\"evt\":\"ERROR\"") != std::string::npos)
                    g_logger.error(stdext::format("[Discord] Error: %s", jsonField(payload, "message")));
                break;
            case OP_PING:
                writeFrame(OP_PONG, payload);
                break;
            case OP_CLOSE:
                g_logger.error(stdext::format("[Discord] Conexion cerrada por Discord: %s", jsonField(payload, "message")));
                disconnect();
                return;
            default:
                break;
        }
    }
}

void Discord::sendPresence(const std::string& details, const std::string& state, const std::string& buttons)
{
    std::string activity = "{";
    activity += "\"details\":" + jsonStr(utf8Truncate(details, 128));
    if (state.size() >= 2)
        activity += ",\"state\":" + jsonStr(utf8Truncate(state, 128));
    activity += stdext::format(",\"timestamps\":{\"start\":%lld}", (long long)START_TIME);

    std::string assets;
    if (std::strlen(RPC_LARGE_IMAGE) > 0)
        assets += "\"large_image\":" + jsonStr(RPC_LARGE_IMAGE);
    if (std::strlen(RPC_LARGE_TEXT) > 0)
        assets += std::string(assets.empty() ? "" : ",") + "\"large_text\":" + jsonStr(RPC_LARGE_TEXT);
    if (!assets.empty())
        activity += ",\"assets\":{" + assets + "}";

    if (!buttons.empty())
        activity += ",\"buttons\":[" + buttons + "]";
    activity += "}";

    const std::string payload = stdext::format(
        "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%d,\"activity\":%s},\"nonce\":\"%d\"}",
        (int)GetCurrentProcessId(), activity, (int)++m_nonce);

    if (writeFrame(OP_FRAME, payload)) {
        m_lastDetails = details;
        m_lastState = state;
        m_lastButtons = buttons;
        m_lastSend = nowSeconds();
    }
}

#endif
