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

#pragma once

#include <framework/discord_config.h>
#include <cstdint>
#include <functional>
#include <string>

#if ENABLE_DISCORD_RPC == 1

// Minimal Discord Rich Presence client speaking the local IPC protocol directly
// (named pipe \\.\pipe\discord-ipc-N). Unlike the deprecated discord-rpc library,
// this supports activity buttons.
class Discord
{
public:
    // canUpdate: player is in game. onUpdate: fills first line (details).
    // onState: fills second line (state); empty falls back to STATE_RPC_TEXT.
    // getName: character name, replaces "{name}" in button URLs (url-encoded).
    void init(std::function<bool()> canUpdate, std::function<void(std::string&)> onUpdate,
              std::function<void(std::string&)> onState = nullptr,
              std::function<std::string()> getName = nullptr);
    void terminate();

private:
    void poll();
    bool connect();
    void disconnect();
    void sendPresence(const std::string& details, const std::string& state, const std::string& buttons);
    bool writeFrame(uint32_t opcode, const std::string& payload);
    void readFrames();

    std::function<bool()> m_canUpdate;
    std::function<void(std::string&)> m_onUpdate;
    std::function<void(std::string&)> m_onState;
    std::function<std::string()> m_getName;

    void* m_pipe = nullptr;
    bool m_initialized = false;
    std::string m_lastDetails;
    std::string m_lastState;
    std::string m_lastButtons;
    int64_t m_lastSend = 0;
    int64_t m_lastConnectAttempt = 0;
    uint64_t m_nonce = 0;
};

extern Discord g_discord;

#endif
