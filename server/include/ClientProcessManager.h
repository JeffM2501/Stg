#pragma once

#include "ConnectedClient.h"

namespace ClientProcessManager
{
	void SetClientState(ConnectedClient& client, ClientState state);

	void Process();
}