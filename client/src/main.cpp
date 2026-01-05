#include "external/fix_win32_compatibility.h"

#include "raylib.h"

#include "game.h"   // an external header in this project
#include "engine_core.h"

#include "connection.h"

using namespace EngineCore;

Scene::Ptr GameScene;

void GameInit()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "STG");
	SetTargetFPS(144);

	EngineCore::Init();
	Connection::Init();

	GameStageManager::SetStage(GameStage::Init);

	// load resources
}

void GameCleanup()
{
	Connection::Cleanup();
	if (GameScene)
	{
		GameScene.release();
	}
	CloseWindow();
}

bool GameUpdate()
{
	Connection::ServiceNetwork();

	if (GameScene)
	{
		if (IsWindowResized())
			GameScene->Resize(GetScreenWidth(), GetScreenHeight());

		GameScene->Update();
	}

	return true;
}

void GameDraw()
{
	ClearBackground(DARKGRAY);

	if (GameScene)
	{
		GameScene->Draw();

		RenderTexture& sceneTexture = GameScene->GetRenderTarget();

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
	GameScene.release();

	if (stage == GameStage::Exiting)
	{
		GameStageManager::Quit();
		return;
	}

	GameScene = EngineCore::CreateScene();

	// common registration

	switch (stage)
	{
	case GameStage::Init:
		// resource load
		GameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				GameStageManager::SetStage(GameStage::Connecting);
			});
		break;

	case GameStage::MainMenu:
		break;

	case GameStage::Connecting:
		Connection::Connect();
		GameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				Connection::ServiceNetwork();
				if (Connection::IsConnected())
					GameStageManager::SetStage(GameStage::InGame);
			});

		GameScene->AddTask(SceneTaskLevel::Draw2d, [](float /*frameTime*/, Scene& /*scene*/)
			{
				// todo register a proper ui system
				DrawText("GameStage::Connecting", 10, 10, 20, BLACK);
				DrawText("Not connected", 10, 40, 20, RED);
			});
		break;

	case GameStage::InGame:

		GameScene->AddTask(SceneTaskLevel::PreUpdate, [](float /*frameTime*/, Scene& /*scene*/)
			{
				Connection::ServiceNetwork();
				if (!Connection::IsConnected())
					GameStageManager::SetStage(GameStage::Exiting);
			});

		GameScene->AddTask(SceneTaskLevel::Draw2d, [](float /*frameTime*/, Scene& /*scene*/)
			{
				DrawText("GameStage::InGame", 10, 40, 20, BLACK);
				DrawText(TextFormat("Connected to server %u", Connection::GetClientId()), 10, 10, 20, GREEN);

				int y = 370;
				for (const auto& msg : Connection::GetServerChat())
				{
					DrawText(msg.c_str(), 10, y, 20, BLACK);
					y += 30;
				}
			});
		break;
	}

	GameScene->Resize(GetScreenWidth(), GetScreenHeight());
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

		if (GameScene)
			GameScene->BeginFrame();

		if (!GameUpdate())
			break;

		GameDraw();

		if (GameScene)
			GameScene->EndFrame();

		EndDrawing();
	}
	GameCleanup();

	return 0;
}
