#include "GameApplication.h"

GameApplication::GameApplication(GLsizei _screenWidth, GLsizei _screenHeight)
	: screenWidth(_screenWidth), screenHeight(_screenHeight), nbFrames(0), finishedMenu(_screenWidth, _screenHeight)
{
	lastTime = glfwGetTime();

	racingScene = new RacingScene();
	mainMenu = new MainMenu();
}

GameApplication::~GameApplication()
{
	delete racingScene;
	delete mainMenu;
	delete crtShader;
}

void GameApplication::Init()
{
	racingScene->InitalizeScene(screenWidth, screenHeight);
	mainMenu->InitScene();
	finishedMenu.Init();

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
								 // Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	// Setup screen VAO
	GLuint quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);

	SetUpBuffers(screenWidth, screenHeight);

	msPerFrameText.Load("Resources/Fonts/arial.ttf");

	crtShader = g_resourceMgr.GetShader(SID("CRT_Shader"));
}


void GameApplication::SetUpBuffers(GLsizei screenWidth, GLsizei screenHeight)
{
	// Framebuffers
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// Create a color attachment texture
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameApplication::Update(GLfloat deltaTime)
{
	// ms per frame
	float currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0)
	{
		msPerFrame = 1000.0 / float(nbFrames);
		nbFrames = 0;
		lastTime += 1.0;
	}

	crtShader->Use();
	glUniform1f(glGetUniformLocation(crtShader->Program, "time"), currentTime);

	switch (currentGameState)
	{
		case GameState::splashScreen:
		{
			splashScreen.Update(deltaTime);

			if (splashScreen.changeScene)
			{
				currentGameState = GameState::mainMenu;
			}

			break;
		}

		case GameState::mainMenu:
		{
			mainMenu->UpdateScene(deltaTime, screenWidth, screenHeight);
			if (mainMenu->playGame)
			{
				currentGameState = GameState::inGame;
				racingScene->finishedRace = false;
				racingScene->sceneBlur = false;
				racingScene->SetPlayerMovement(true);
				racingScene->SetRenderUIState(true);
				racingScene->player.recordRace = true;
			}
			break;
		}

		case GameState::inGame:
		{
			racingScene->UpdateScene(deltaTime);

			if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_ESCAPE)
				|| InputManager::GetInstance().IsControllerButtonPressed(XBOX360_BACK))
			{
				currentGameState = GameState::mainMenu;
				racingScene->sceneBlur = true;
				racingScene->SetPlayerMovement(false);
				racingScene->SetRenderUIState(false);
				racingScene->player.recordRace = false;
			}

			if (racingScene->finishedRace)
			{
				currentGameState = GameState::finishedMenu;
				racingScene->sceneBlur = true;
				racingScene->SetPlayerMovement(false);
				racingScene->SetRenderUIState(false);
				racingScene->player.recordRace = false;
				finishedMenu.finishedLapTime = racingScene->currentTime;
			}

			break;
		}

		case GameState::finishedMenu:
		{
			racingScene->UpdateScene(deltaTime);
			finishedMenu.Update();

			if (finishedMenu.selectedMainMenu)
			{
				currentGameState = GameState::mainMenu;
				racingScene->player.Spawn();
				finishedMenu.selectedMainMenu = false;
				racingScene->player.WriteRecordedPositions();
				racingScene->bestTime = finishedMenu.personalBestTime;
				racingScene->ResetScene();
			}
			else if (finishedMenu.selectedPlayAgain)
			{
				currentGameState = GameState::inGame;
				racingScene->player.Spawn();
				racingScene->finishedRace = false;
				racingScene->sceneBlur = false;
				racingScene->SetPlayerMovement(true);
				racingScene->SetRenderUIState(true);
				finishedMenu.selectedPlayAgain = false;
				racingScene->player.WriteRecordedPositions();
				racingScene->bestTime = finishedMenu.personalBestTime;
				racingScene->ResetScene();
			}

			break;
		}
	}
}

void GameApplication::Render(bool windowResized)
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stringstream ss;
	ss << "ms per frame: " << msPerFrame;
	msPerFrameText.RenderText(ss.str(), Vector2(10.0f, screenHeight - 30.0f), 0.4f, Vector3(0.0, 1.0f, 0.0f), screenWidth, screenHeight);
	glDisable(GL_BLEND);

	if (windowResized)
		SetUpBuffers(screenWidth, screenHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (currentGameState)
	{
		case GameState::splashScreen:
		{
			splashScreen.Render(screenWidth, screenHeight);
			break;
		}

		case GameState::mainMenu:
		{
			mainMenu->RenderScene(screenWidth, screenHeight);
			racingScene->RenderScene(screenWidth, screenHeight);
			break;
		}

		case GameState::inGame:
		{
			racingScene->RenderScene(screenWidth, screenHeight);
			break;
		}

		case GameState::finishedMenu:
		{
			finishedMenu.Render(screenWidth, screenHeight);
			racingScene->RenderScene(screenWidth, screenHeight);
			break;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw Screen
	crtShader->Use();
	glUniform2f(glGetUniformLocation(crtShader->Program, "resolution"), screenWidth, screenHeight);
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

bool GameApplication::GetCloseState()
{
	return mainMenu->exitGame;
}