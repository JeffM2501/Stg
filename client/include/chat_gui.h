#pragma once

#include "raylib.h"
#include "engine_core.h"

#include "lifetime_token.h"

#include <string>

class ChatGUI
{
public:
    ChatGUI(const Rectangle& rect, EngineCore::Scene* scene);
    ~ChatGUI();

    bool WantKeyInput() const;

    void Update();
    void Render();

private:
    Rectangle Bounds;
    EngineCore::Scene* GameScene;

    Tokens::TokenSource Token;
    std::vector<uint32_t> ChatGroups;

    uint32_t CurrentGroup = 0;

    bool InputMode = false;

    std::string CurrentInput;
    void ProcessInput();
    void SendMessage();

    bool BlinkOn = false;
    float BlinkAccumulator = 0.0f;
    float BinkInterval = 0.5f;
};