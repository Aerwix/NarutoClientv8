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

// DISCORD RICH PRESENCE (https://discord.com/developers/applications)
// Implementacion propia del IPC de Discord (sin librerias externas), ver framework/discord/.
// Solo Windows: en Linux/Android queda deshabilitado.
#if defined(_WIN32) && !defined(ANDROID)
#define ENABLE_DISCORD_RPC 1 // 1 = habilitado | 0 = deshabilitado
#else
#define ENABLE_DISCORD_RPC 0
#endif

#define RPC_API_KEY "1200477724066717876" // Application ID de tu app en el Discord Developer Portal

#define SHOW_CHARACTER_NAME_RPC 1  // 1 = mostrar nombre del personaje
#define SHOW_CHARACTER_LEVEL_RPC 1 // 1 = mostrar level
#define SHOW_CHARACTER_WORLD_RPC 1 // 1 = mostrar mundo

#define SHOW_CHARACTER_VOCATION_RPC 1 // 1 = segunda linea muestra la vocacion
#define SHOW_CHARACTER_GUILD_RPC 1    // 1 = segunda linea muestra la guild
// Segunda linea: "Vocacion • Guild". Si no hay ninguna de las dos usa STATE_RPC_TEXT.

// Separador entre datos (Nombre (Lv) • Mundo / Vocacion • Guild).
// "\xE2\x80\xA2" = • en UTF-8 (escrito como escape para no depender de la codificacion del archivo)
#define RPC_SEPARATOR " \xE2\x80\xA2 "

// Nombres de vocacion por clientid (debe coincidir con clientid en vocations.xml del servidor).
// El indice es el clientid. Si agregas vocaciones en el servidor, agregalas aqui en orden.
static const char* const RPC_VOCATION_NAMES[] = {
    "",           // 0  None
    "Naruto",     // 1
    "Sakura",     // 2
    "Sasuke",     // 3
    "Hinata",     // 4
    "Kiba",       // 5
    "Shino",      // 6
    "Shikamaru",  // 7
    "Ino",        // 8
    "Chouji",     // 9
    "Rock Lee",   // 10
    "Neji",       // 11
    "Tenten",     // 12
    "Gaara",      // 13
    "Temari",     // 14
    "Kankuro",    // 15
    "Pain",       // 16
    "Konan",      // 17
    "Itachi",     // 18
    "Kisame",     // 19
    "Deidara",    // 20
    "Sasori",     // 21
    "Hidan",      // 22
    "Kakuzu",     // 23
    "Zetsu",      // 24
    "Obito",      // 25
    "Jiraiya",    // 26
    "Orochimaru", // 27
    "Tsunade",    // 28
    "Haku",       // 29
    "Zabuza",     // 30
};

#define OFFLINE_RPC_TEXT "Selecting Character..." // Texto cuando no hay personaje logueado
#define STATE_RPC_TEXT "aerwix.com"               // Segunda linea (state)
#define RPC_LARGE_IMAGE "rpc-logo"                // Key de la imagen subida en Rich Presence > Art Assets
#define RPC_LARGE_TEXT "Naruto Shadow Rising"     // Tooltip de la imagen grande
#define RPC_UPDATE_INTERVAL 15000                 // ms entre reenvios (cambios se envian al instante)

// Botones (maximo 2). Label max 32 caracteres, URL debe empezar con https://
// Deja label o url vacio ("") para ocultar el boton.
#define RPC_BUTTON1_LABEL "Create your account!"
#define RPC_BUTTON1_URL "https://aerwix.com/index.php/account/create"
// {name} se reemplaza por el nombre del personaje (ej: Zabuza+Godmochi).
// Si la URL usa {name}, el boton solo aparece con un personaje logueado.
#define RPC_BUTTON2_LABEL "Check Character"
#define RPC_BUTTON2_URL "https://aerwix.com/index.php/characters/{name}"
