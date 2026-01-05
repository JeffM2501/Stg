#pragma once

#include "connected_client.h"

namespace ClientProcessManager
{
    void SetClientState(ConnectedClient& client, ClientState state);

    void Process();
}