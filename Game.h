#pragma once
//vendor
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//c++ std 
#include <iostream>
#include <optional>

//custom
#include "GameContext.h"
#include "Rendering/Renderer.h"
#include "World.h"
#include "Player.h"
#include "Keyboard.h"
#include "Mouse.h"

class Game
{
public:
	struct GameServices
	{
		MT::EventSystem<GameEventPolicy> gameEvents;
		MT::ThreadPool threadPool;

		GameServices() = default;

		GameServices(const GameServices& other) = delete;
		GameServices& operator=(const GameServices& other) = delete;

		GameServices(GameServices&& other) = default;
		GameServices& operator=(GameServices&& other) = default;
	};

private:

	GameServices m_gameServices;

	GLFWwindow* m_window;

	Renderer m_renderer;
	std::optional<World> m_world;
	Keyboard m_keyboard;
	Mouse m_mouse;

	float m_frameTime;
	float m_aspectRatio, m_fov /*degrees*/;
	int m_width, m_height;
	/*std::queue<std::pair<int, int>> keyPressQueue;*/

	void processInputs();

	//static callbacks
	static void static_keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void static_windowResizeCallback(GLFWwindow* window, int width, int height);
	static void static_mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
	static void static_mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void static_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	//callbacks
	void keyPressCallback(int key, int scancode, int action, int mods);
	void windowResizeCallback(int width, int height);
	void mouseMoveCallback(double xpos, double ypos);
	void mouseScrollCallback(double xoffset, double yoffset);
	void mouseButtonCallback(int button, int action, int mods);

	//key binds
	void close();

public:

	Game();
	~Game();

	Game(const Game&) = delete;
	Game operator=(const Game&) = delete;

	int run();

};

