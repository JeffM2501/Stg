#include "external/fix_win32_compatibility.h"

#include "raylib.h"

#include "game.h"   // an external header in this project
#include "engine_core.h"

#include "connection.h"
#include "chat_client.h"

#include "chat_gui.h"

using namespace EngineCore;


class GameScene : public EngineCore::Scene
{
	public:
	virtual ~GameScene() {}

    std::unique_ptr<ChatGUI> ChatInterface;

protected:
	void OnInit() override
	{
	
	}
	
	void OnRegisterComponents() override
	{
	}
	
	void OnRegisterTasks() override
	{
       ChatInterface = std::make_unique<ChatGUI>(Rectangle{ 10, 10, GetScreenWidth()/2.0f, 200 }, this);
    }
};

std::unique_ptr<GameScene> pGameScene;


void GameInit()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "STG");
	SetTargetFPS(144);

	ChatClient::Init();
	EngineCore::Init();
	Connection::Init();

	GameStageManager::SetStage(GameStage::Init);

	// load resources
}

void GameCleanup()
{
	Connection::Cleanup();
	if (pGameScene)
	{
		pGameScene.release();
	}
	CloseWindow();
}

bool GameUpdate()
{
	Connection::ServiceNetwork();
	ChatClient::Process();

	if (pGameScene)
	{
		if (IsWindowResized())
			pGameScene->Resize(GetScreenWidth(), GetScreenHeight());

		pGameScene->Update();
	}

	return true;
}

void GameDraw()
{
	ClearBackground(DARKGRAY);

	if (pGameScene)
	{
		pGameScene->Draw();

		RenderTexture& sceneTexture = pGameScene->GetRenderTarget();

		float scale = float(GetScreenWidth()) / float(sceneTexture.texture.width);

		Rectangle targetRect = { 0, 0, float(GetScreenWidth()), float(sceneTexture.texture.height) * scale };
		if (targetRect.height > float(GetScreenHeight()))
		{
			scale = float(GetScreenHeight()) / float(sceneTexture.texture.height);
			targetRect = { 0, 0,float(GetScreenWidth()) * scale, float(GetScreenHeight()) };
		}

		targetRect.x = (float(GetScreenWidth()) - targetRect.width) / 2;
		targetRect.y = (float(GetScreenHeight()) - targetRect.height) / 2;

		DrawTexturePro(sceneTexture.texture,
			{ 0, 0, float(sceneTexture.texture.width), float(-sceneTexture.texture.height) },
			targetRect,
			{ 0, 0 },
			0.0f,
			WHITE);
	}
}

bool Quit()
{
	return WindowShouldClose() || GameStageManager::WantExit();
}

void OnStageChanged(void* sender, const GameStage& stage)
{
	pGameScene.release();

	if (stage == GameStage::Exiting)
	{
		GameStageManager::Quit();
		return;
	}

	pGameScene = EngineCore::CreateScene<GameScene>();

	// common registration

	switch (stage)
	{
	case GameStage::Init:
		// resource load
		pGameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				GameStageManager::SetStage(GameStage::Connecting);
			});
		break;

	case GameStage::MainMenu:
		break;

	case GameStage::Connecting:
		Connection::Connect();
		pGameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				Connection::ServiceNetwork();
				if (Connection::IsConnected())
					GameStageManager::SetStage(GameStage::InGame);
			});

		pGameScene->AddTask(SceneTaskLevel::Draw2d, [](float /*frameTime*/, Scene& /*scene*/)
			{
				// todo register a proper ui system
				DrawText("GameStage::Connecting", 10, 10, 20, BLACK);
				DrawText("Not connected", 10, 40, 20, RED);
			});
		break;

	case GameStage::InGame:

		pGameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				Connection::ServiceNetwork();
				if (!Connection::IsConnected())
					GameStageManager::SetStage(GameStage::Exiting);
			});

 		pGameScene->AddTask(SceneTaskLevel::Draw2d, [](float /*frameTime*/, Scene& /*scene*/)
 			{
 				DrawText("GameStage::InGame", 10, 40, 20, BLACK);
 				DrawText(TextFormat("Connected to server %u", Connection::GetClientId()), 10, 10, 20, GREEN);
 
 				auto user = ChatClient::GetUserFromId(Connection::GetClientId());
 				if (user)
 					DrawText(TextFormat("Player Name: %s", user->Name.data()), 10, 60, 20, GRAY);
 
//  				int y = 370;
//  				for (const auto& msg : ChatClient::GetChatLog())
//  				{
//  					std::string text;
//  					auto* user = ChatClient::GetUserFromId(msg.SenderId);
//  					if (!user)
//  						text += "[server]:";
//  					else
//  						text += "[" + user->Name + "]:";
//  
//  					text += msg.Message;
//  
//  					DrawText(text.c_str(), 10, y, 20, BLACK);
//  					y += 30;
//  				}
 			});
		break;
	}

	pGameScene->Resize(GetScreenWidth(), GetScreenHeight());
}

int main()
{
	SetConfigFlags(FLAG_WINDOW_HIGHDPI);
	Tokens::TokenSource mainLifeitmeToken;
	GameStageManager::OnStageChanged.Add(OnStageChanged, mainLifeitmeToken.GetToken());

	GameInit();
	GameStageManager::ApplyPendingStage();

	GameStageManager::SetStage(GameStage::Connecting);

	while (!Quit())
	{
		BeginDrawing();
		GameStageManager::ApplyPendingStage();

		if (pGameScene)
			pGameScene->BeginFrame();

		if (!GameUpdate())
			break;

		GameDraw();

		if (pGameScene)
			pGameScene->EndFrame();

		EndDrawing();
	}
	GameCleanup();

	return 0;
}
