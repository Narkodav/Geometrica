#include "Game.h"

//static callbacks

void Game::static_keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static_cast<Game*>(glfwGetWindowUserPointer(window))->keyPressCallback(key, scancode, action, mods);
}

void Game::static_windowResizeCallback(GLFWwindow* window, int width, int height)
{
    static_cast<Game*>(glfwGetWindowUserPointer(window))->windowResizeCallback(width, height);
}

void Game::static_mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    static_cast<Game*>(glfwGetWindowUserPointer(window))->mouseMoveCallback(xpos, ypos);
}

void Game::static_mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    static_cast<Game*>(glfwGetWindowUserPointer(window))->mouseScrollCallback(xoffset, yoffset);
}

void Game::static_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    static_cast<Game*>(glfwGetWindowUserPointer(window))->mouseButtonCallback(button, action, mods);
}

//callbacks
void Game::windowResizeCallback(int width, int height)
{
    m_width = width;
    m_height = height;
    m_aspectRatio = (double)m_width / m_height;
    m_renderer.setProjection(m_aspectRatio, m_fov);
    m_renderer.resetViewport(m_width, m_height);
    glfwSetCursorPos(m_window, 0, 0);
}

void Game::mouseMoveCallback(double xpos, double ypos)
{
    m_mouse.updateMouseMovement(xpos, ypos, m_frameTime);
    glfwSetCursorPos(m_window, 0, 0);
}

void Game::mouseScrollCallback(double xoffset, double yoffset)
{
    m_mouse.offsetY = yoffset;
    //m_fov -= yoffset * m_scrollSensitivity;
    //m_renderer->setProjection(m_aspectRatio, m_fov);
}

#define CHECK_KEY(glfwCode, customCode, virtualKeyState)     case glfwCode: \
m_keyboard.m_keys[customCode].isChanged = (virtualKeyState != m_keyboard.m_keys[customCode].state); \
m_keyboard.m_keys[customCode].state = virtualKeyState; \
break \

void Game::keyPressCallback(int key, int scancode, int action, int mods)
{
    bool virtualKeyState = (action != GLFW_RELEASE);
    switch (key)
    {
        CHECK_KEY(GLFW_KEY_W, KEY_W, virtualKeyState);
        CHECK_KEY(GLFW_KEY_S, KEY_S, virtualKeyState);
        CHECK_KEY(GLFW_KEY_A, KEY_A, virtualKeyState);
        CHECK_KEY(GLFW_KEY_D, KEY_D, virtualKeyState);
        CHECK_KEY(GLFW_KEY_LEFT_SHIFT, KEY_SHIFT, virtualKeyState);
        CHECK_KEY(GLFW_KEY_SPACE, KEY_SPACE, virtualKeyState);
        CHECK_KEY(GLFW_KEY_ESCAPE, KEY_ESC, virtualKeyState);
    }
}

void Game::mouseButtonCallback(int button, int action, int mods)
{

    bool virtualKeyState = (action == GLFW_PRESS);
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            m_mouse.LMB.isChanged = (virtualKeyState != m_mouse.LMB.state);
            m_mouse.LMB.state = virtualKeyState;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            m_mouse.RMB.isChanged = (virtualKeyState != m_mouse.RMB.state);
            m_mouse.RMB.state = virtualKeyState;
            break;
        case GLFW_MOUSE_BUTTON_3:
            m_mouse.scrollWheel.isChanged = (virtualKeyState != m_mouse.scrollWheel.state);
            m_mouse.scrollWheel.state = virtualKeyState;
            break;
    }
}

void Game::processInputs()
{
    if(m_keyboard.m_keys[KEY_ESC].isChanged)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);

    if (m_keyboard.m_keys[KEY_PLAYER_ACTION_FLAG].isChanged)
    {
        m_keyboard.m_keys[KEY_PLAYER_ACTION_FLAG].state = 1;
    }

    m_world.value().handleInputs(m_mouse, m_keyboard, constDeltaTime); //mouse for LMB and RMB
    for (int i = 0; i < KEY_COUNT; i++)
        m_keyboard.m_keys[i].isChanged = false;
    m_mouse.LMB.isChanged = false;
    m_mouse.RMB.isChanged = false;
    m_mouse.scrollWheel.isChanged = false;
}

Game::Game() : m_frameTime(0.f),
m_fov(120.f), m_width(1000), m_height(800),
m_renderer(GameContext(m_gameServices.gameEvents, m_gameServices.threadPool))
{
    m_mouse.mouseSensitivity = 0.01f;
    m_mouse.scrollSensitivity = 2.0f;
    if (!glfwInit())
    {
        std::cerr << "failed to load glfw" << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif // __DEBUG

    /* Create a windowed mode window and its OpenGL context */
    m_window = glfwCreateWindow(m_width, m_height, "test", NULL, NULL);

    m_aspectRatio = (float)m_width / m_height;

    if (m_window == NULL)
    {
        glfwTerminate();
        std::cerr << "failed to create a window" << std::endl;
    }

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(m_window, 0, 0);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, static_keyPressCallback);
    glfwSetWindowSizeCallback(m_window, static_windowResizeCallback);
    glfwSetCursorPosCallback(m_window, static_mouseMoveCallback);
    glfwSetScrollCallback(m_window, static_mouseScrollCallback);
    glfwSetMouseButtonCallback(m_window, static_mouseButtonCallback);
}

Game::~Game()
{
    glfwTerminate();
}

void Game::close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

int Game::run()
{
    int loadDistance = 10;
    int numOfAllocatedThreads = /*std::thread::hardware_concurrency() * 0.7*/ 32;
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK)
        std::cout << "glew failed to initiate" << std::endl;
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << version << std::endl;

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    glDebugMessageCallback(Renderer::static_glDebugOutput, nullptr);
#endif //_DEBUG

    DataRepository::set("res/resourcePack");
    m_gameServices.threadPool.init(16);
    m_world.emplace(loadDistance, 1234, Player(0.f, 0.f, glm::vec3(2.f, 205.f, 2.f), glm::vec3(0.f), 
    GameContext(m_gameServices.gameEvents, m_gameServices.threadPool)),
        GameContext(m_gameServices.gameEvents, m_gameServices.threadPool));
    m_renderer.set(DEPTH_TESTING | FACE_CULLING | BLENDING, loadDistance, 
    m_aspectRatio, m_fov, m_window, &m_world.value().getChunkManager().getChunkMap());
   
    
    float accumulator = 0.f;
    float runTime = 0.f;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float frameRateUpdateCounter = 0.f;

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        processInputs();

        auto newTime = std::chrono::high_resolution_clock::now();
        m_frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(newTime - currentTime).count();
        currentTime = newTime;
        if (m_frameTime > constDeltaTime * 3)
            m_frameTime = constDeltaTime * 3;
        accumulator += m_frameTime;

        while (accumulator >= constDeltaTime)
        {
            m_world.value().physicsUpdate(constDeltaTime);
            accumulator -= constDeltaTime;
            runTime += constDeltaTime;
        }

        if (runTime >= frameRateUpdateCounter)
        {
            m_renderer.updateFrameRate();
            frameRateUpdateCounter += 0.5;
        }
        m_world.value().iterate();
        m_renderer.update({ accumulator / constDeltaTime,
        m_world.value().getPlayer().getPosition(),
        m_world.value().getPlayer().getVelocity(),
        m_world.value().getPlayer().getAcceleration(),
        m_world.value().getPlayer().getViewWithoutTranspos(),
        m_world.value().getPlayer().getView(),
        m_world.value().getPlayer().getChunkCoords(),
        m_world.value().getPlayer().getRaycastResult(),
        m_world.value().getPlayer().getSelectedArea() });

#ifdef _DEBUG
        /*m_gameServices.threadPool.checkForDeadlocks();*/
#endif
    }

    m_renderer.terminate();

    //Logger::displayLog();

    return 0;
}

