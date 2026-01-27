
#include "chat_gui.h"
#include "chat_client.h"

ChatGUI::ChatGUI(const Rectangle& rect, EngineCore::Scene* scene)
    : Bounds(rect)
    , GameScene(scene)
{
    ChatClient::OnChannelAdded.Add([this](void* /*sender*/, uint32_t groupID)
        {
            ChatGroups.push_back(groupID);
        }, Token.GetToken());

    ChatGroups.push_back(0);
    CurrentGroup = 0;

    scene->AddTask(EngineCore::SceneTaskLevel::PreDraw2d, [this](float /*dt*/, EngineCore::Scene& /*scene*/)
        {
            this->Update();
        }, true, nullptr);

    scene->AddTask(EngineCore::SceneTaskLevel::Draw2d, [this](float /*dt*/, EngineCore::Scene& /*scene*/)
        {
            this->Render();
        }, true, nullptr);
}

ChatGUI::~ChatGUI()
{
}

bool ChatGUI::WantKeyInput() const
{
    return InputMode;
}

void ChatGUI::Update()
{
    if (InputMode)
    {
        ProcessInput();
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        {
            InputMode = true;
        }
    }
    BlinkAccumulator += GetFrameTime();
    while (BlinkAccumulator >= BinkInterval)
     {
        BlinkAccumulator -= BinkInterval;
        BlinkOn = !BlinkOn;
    }
}

void ChatGUI::Render()
{
    Rectangle chatArea = Bounds;
    chatArea.y = GetScreenHeight() - (Bounds.y + Bounds.height);
    Rectangle inputRect = { chatArea.x + 5, chatArea.y + chatArea.height - 30, chatArea.width - 10, 25 };

    Rectangle headerRect = chatArea;
    headerRect.height = 20;

    DrawRectangleRec(chatArea, ColorAlpha(GRAY, 0.5f));
    DrawRectangleRec(inputRect, ColorAlpha(BLACK, 0.5f));
    DrawRectangleRec(headerRect, ColorAlpha(WHITE, 0.5f));

    float groupWidth = 100.0f;
    for (int i = 0; i < ChatGroups.size(); i++)
    {
        Rectangle groupRect = { chatArea.x + i * groupWidth, chatArea.y, groupWidth - 2, headerRect.height };

        if (groupRect.x + groupRect.width > chatArea.x + chatArea.width)
            break;
        Color textColor = (ChatGroups[i] == CurrentGroup) ? BLUE : WHITE;
        DrawRectangleRec(groupRect, ColorAlpha(textColor, 0.5f));
        auto* group = ChatClient::GetGroup(ChatGroups[i]);
        std::string groupName = group ? group->Name : "Unknown";
 
        DrawText(groupName.c_str(), int(groupRect.x + 5), int(groupRect.y), 20, BLACK);

        Rectangle chatRect = { chatArea.x, groupRect.y + groupRect.height, chatArea.width, inputRect.y - (groupRect.y + groupRect.height) };

        BeginScissorMode(int(chatRect.x), int(chatRect.y), int(chatRect.width), int(chatRect.height));
        float chatRectBottom = chatRect.y + chatRect.height;
        if (i == CurrentGroup)
        {
            for (auto messageItr = group->ChatLog.rbegin(); messageItr != group->ChatLog.rend(); messageItr++)
            {
                auto& message = *messageItr;

                Color textColor = WHITE;
                const char* text = nullptr;
                auto* user = ChatClient::GetUserFromId(message.SenderId);
                if (!user)
                {
                    textColor = YELLOW;
                    text = TextFormat("Server:%s", message.Message.c_str());
                }
                else
                {
                    if (message.SenderId == ChatClient::GetUserID())
                        textColor = SKYBLUE;

                    text = TextFormat("%s:%s", user->Name.c_str(), message.Message.c_str());
                }
                DrawText(text, int(chatRect.x), int(chatRectBottom - 20), 20, textColor);

                chatRectBottom -= 20;

                if (chatRectBottom <= chatRect.y)
                    break;
            }
        }
        EndScissorMode();
    }
   
    if (InputMode)
    {
        DrawText(CurrentInput.c_str(), int(inputRect.x + 5), int(inputRect.y + 2), 20, WHITE);

        DrawRectangleLinesEx(inputRect, 2, DARKBLUE);

        Rectangle cursorRect = { inputRect.x + MeasureText(CurrentInput.c_str(), 20) + 4, inputRect.y + 2, 20, 20 };
        if (BlinkOn)
            DrawRectangleRec(cursorRect, DARKBLUE);
    }
}

void ChatGUI::ProcessInput()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        InputMode = false;
        CurrentInput.clear();
        return;
    }

    while (int key = GetCharPressed())
    {
        CurrentInput += (char)key;
    }

    while (int key = GetKeyPressed())
    {
        if (key == KEY_BACKSPACE)
        {
            if (!CurrentInput.empty())
                CurrentInput.resize(CurrentInput.size() - 1);
        }
        else if (key == KEY_TAB)
        {
            if (!CurrentInput.empty())
            {
                // TODO, auto complete?
            }
        }
        else if (key == KEY_ENTER || key == KEY_KP_ENTER)
        {
            if (!CurrentInput.empty())
            {
                SendMessage();
                CurrentInput.clear();
                InputMode = false;
            }
        }
    }
}

void ChatGUI::SendMessage()
{
    if (CurrentInput.empty())
        return;

    ChatClient::Send(CurrentGroup, CurrentInput);
}
