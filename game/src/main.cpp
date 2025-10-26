/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "external/fix_win32_compatibility.h"

#include "raylib.h"
#include "raymath.h"

#include "game.h"   // an external header in this project
#include "rlgl.h"	// an external header in the static lib project

#define ENET_IMPLEMENTATION
#include "enet.h"
#undef ENET_IMPLEMENTATION

#include "MessageChannels.h"

#include <string>

ENetHost* Client = { 0 };
ENetPeer* ServerPeer = { 0 };
bool Connected = false;
bool WantExit = false;

bool Connect(std::string_view host = "127.0.0.1")
{
    ENetAddress address = { 0 };
    ENetEvent event = { 0 };
    ENetPeer* peer = { 0 };

    enet_address_set_host(&address, host.data());
    address.port = 7777;
    ServerPeer = enet_host_connect(Client, &address, int(NetworkChannelIDs::Count), 0);

    return ServerPeer != nullptr;
}

void ServiceNetwork()
{
    if (Client == nullptr)
        return;

    ENetEvent event = {};
    if (enet_host_service(Client, &event, 10))
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            Connected = true;
            // connected to server
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            // received a packet from server
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
        case ENET_EVENT_TYPE_DISCONNECT:
            // disconnected from server
            WantExit = true;
            Connected = false;
            enet_host_destroy(Client);
            Client = nullptr;
            break;
        default:
            break;
        }
    }
}

void GameInit()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(InitialWidth, InitialHeight, "STG");
    SetTargetFPS(144);

    enet_initialize();

    Client = enet_host_create(NULL, 1 , int(NetworkChannelIDs::Count), 0 ,0);

    // load resources
}

void GameCleanup()
{
    // unload resources
    if (Client)
    {
        enet_host_destroy(Client);
        Client = nullptr;
    }

    CloseWindow();
}

bool GameUpdate()
{
    ServiceNetwork();
    return true;
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawText("STG", 10, 10, 20, WHITE);

    if (Connected)
        DrawText("Connected to server", 10, 40, 20, GREEN);
    else
        DrawText("Not connected", 10, 40, 20, RED);
    EndDrawing();
}

bool Quit()
{
    return WindowShouldClose() || WantExit;
}

int main()
{
    GameInit();

    Connect();

    while (!Quit())
    {
        if (!GameUpdate())
            break;

        GameDraw();
    }
    GameCleanup();

    return 0;
}