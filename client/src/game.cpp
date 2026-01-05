#include "external/fix_win32_compatibility.h"

#include "Game.h"

#include <mutex>

namespace GameStageManager
{
	Events::EventSource<GameStage> OnStageChanged;
	static GameStage CurrentStage = GameStage::None;

	static GameStage PendingStage = GameStage::None;

	static bool WantQuit = false;
	static std::mutex StageMutex;

	void SetStage(GameStage newStage)
	{
		std::lock_guard<std::mutex> lock(StageMutex);
		if (newStage != PendingStage)
		{
			PendingStage = newStage;
		}
	}

	GameStage GetCurrentStage()
	{
		return CurrentStage;
	}

	void ApplyPendingStage()
	{
		bool changed = false;

		{
			std::lock_guard<std::mutex> lock(StageMutex);
			if (PendingStage != GameStage::None && PendingStage != CurrentStage)
			{
				CurrentStage = PendingStage;
				PendingStage = GameStage::None;
				changed = true;
			}
		}

		if (changed)
			OnStageChanged.Invoke(nullptr, CurrentStage);
	}

	void Quit()
	{
		WantQuit = true;
	}

	bool WantExit()
	{
		return WantQuit;
	}
}