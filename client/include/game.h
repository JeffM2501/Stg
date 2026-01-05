#pragma once

#include "events.h"

enum class GameStage
{
    None,
    Init,
    MainMenu,
    Connecting,
    InGame,
    Exiting
};

namespace GameStageManager
{
    extern Events::EventSource<GameStage> OnStageChanged;

    void SetStage(GameStage newStage);
    GameStage GetCurrentStage();

    void ApplyPendingStage();

    void Quit();
    bool WantExit();
}