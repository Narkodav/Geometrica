#include "Renderer.h"

#ifdef _DEBUG
void APIENTRY Renderer::static_glDebugOutput(unsigned int source, unsigned int type, unsigned int id, unsigned int severity,
    int length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cerr << "---------------" << std::endl;
    std::cerr << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
    } std::cerr << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
    } std::cerr << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; __debugbreak(); break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
    
    } std::cerr << std::endl;
    std::cerr << std::endl;
}
#endif // _DEBUG

Renderer::Renderer(const GameContext& gameContext) :
    m_gameContext(gameContext)
{

}

void Renderer::setForThread(unsigned int parameters, int renderDistance, float aspectRatio, float fov, GLFWwindow* window, const ChunkMap* chunkMapHandle)
{
    m_chunkMapHandle = chunkMapHandle;
    m_windowHandle = window;
    glfwMakeContextCurrent(m_windowHandle);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, 1);
    ImGui_ImplOpenGL3_Init("#version 430");

    m_loadingDataRepository = 0;
    m_shouldStop = 0;
    m_renderDistance = renderDistance;
    setProjection(aspectRatio, fov);

    if (parameters & DEPTH_TESTING)
    {
        m_flags[FLAG_DEPTH_TESTING] = 1;
        glEnable(GL_DEPTH_TEST);
    }
    else m_flags[FLAG_DEPTH_TESTING] = 0;
    if (parameters & FACE_CULLING)
    {
        m_flags[FLAG_FACE_CULLING] = 1;
        glEnable(GL_CULL_FACE);
    }
    else m_flags[FLAG_FACE_CULLING] = 0;
    if (parameters & BLENDING)
    {
        m_flags[FLAG_BLENDING] = 1;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else m_flags[FLAG_BLENDING] = 0;

    m_shaders[SHADER_CUBOID].set("shaders/cuboidFace.shader");
    m_shaders[SHADER_BLOCK].set("shaders/block.shader");
    m_shaders[SHADER_BLOCK_HIGHLIGHT].set("shaders/blockHighlight.shader");
    m_shaders[SHADER_SKYBOX].set("shaders/skybox.shader");
    m_shaders[SHADER_FLAT_TO_SCREEN].set("shaders/FlatToScreen.shader");

    debugBillboard.set();
    m_mesh.emplace(m_renderDistance, m_gameContext, *chunkMapHandle);
    m_mesh.value().set();
    rendererLoop();
}

void Renderer::set(unsigned int parameters, int renderDistance, float aspectRatio, float fov, GLFWwindow* window, const ChunkMap* chunkMapHandle)
{
    glfwMakeContextCurrent(nullptr);
    m_loadingDataRepository = 1;
    m_rendererThread = std::thread(&Renderer::setForThread, this, parameters, renderDistance, aspectRatio, fov, window, chunkMapHandle);
    while (m_loadingDataRepository);
}

void Renderer::toggleDepthTest()
{
    if (m_flags[FLAG_DEPTH_TESTING])
    {
        glDisable(GL_DEPTH_TEST);
        m_flags[FLAG_DEPTH_TESTING] = 0;
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        m_flags[FLAG_DEPTH_TESTING] = 1;
    }
}

void Renderer::toggleFaceCulling()
{
    if (m_flags[FLAG_FACE_CULLING])
    {
        glDisable(GL_CULL_FACE);
        m_flags[FLAG_FACE_CULLING] = 0;
    }
    else
    {
        glEnable(GL_CULL_FACE);
        m_flags[FLAG_FACE_CULLING] = 1;
    }
}

void Renderer::toggleBlending()
{
    if (m_flags[FLAG_BLENDING])
    {
        glDisable(GL_BLEND);
        m_flags[FLAG_BLENDING] = 0;
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_flags[FLAG_BLENDING] = 1;
    }
}

Renderer::~Renderer()
{
    terminate();
}

void Renderer::setProjection(float aspectRatio, float fov)
{
    m_projection = glm::mat4(0.0f);
    m_projection = glm::perspective(glm::radians(fov), aspectRatio, 0.05f, 1000.0f);
    //glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
}

void Renderer::clear() const
{
    if(m_flags[FLAG_DEPTH_TESTING])
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    else glClear(GL_COLOR_BUFFER_BIT);
    
}

void Renderer::terminate()
{
    m_shouldStop = 1;
    while (!m_rendererThread.joinable()) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    m_rendererThread.join();
}

void Renderer::render(RenderParams params)
{
    float delta = constDeltaTime * params.alpha;
    glm::vec3 currentPos = params.position;
    glm::vec3 futurePos = currentPos + params.velocity * delta + params.acceleration * delta * delta * 0.5f;
    glm::vec3 interpolatedDelta = params.alpha * (futurePos - currentPos);
    params.interpolatedView = glm::translate(params.interpolatedView, -interpolatedDelta);
    m_mesh.value().drawSkybox(m_shaders[SHADER_SKYBOX], params.viewWithoutTranspos, m_projection );
    m_mesh.value().drawChunks(m_shaders[SHADER_CUBOID], m_shaders[SHADER_BLOCK], params.interpolatedView, m_projection);
    m_mesh.value().drawBlockHighlight(m_shaders[SHADER_BLOCK_HIGHLIGHT], params.interpolatedView, m_projection, params.selectedArea);
    

}

void Renderer::debugRender()
{
    /*render();*/
    
}

void Renderer::rendererLoop()
{
    std::chrono::steady_clock clock;
    float frameStart = 0;
    float frameEnd = 0;
    RenderParams renderCopy;
    m_frameRateCalculator.setFrameTimeBuffer(240);
    glfwSwapInterval(0);

    while (!m_shouldStop.load())
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            renderCopy = m_renderParams;
        }
        if (m_shouldResetViewport.load())
        {
            m_shouldResetViewport.store(0);
            glViewport(0, 0, m_newWidth, m_newHeight);
        }

        m_isRendering.store(1);
        m_shouldRender.store(0);
        frameStart = std::chrono::duration<float>(clock.now().time_since_epoch()).count();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderCornerDebugOverlay(renderCopy);
        displayChunkOverlay(renderCopy);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_windowHandle, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        clear();
        render(renderCopy);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_windowHandle);

        m_mesh.value().iterate(renderCopy.chunkCoords);
        m_mesh.value().processBuffers();

        frameEnd = std::chrono::duration<float>(clock.now().time_since_epoch()).count();
        m_frameRateCalculator.addFrameTime(frameEnd - frameStart);
        m_isRendering.store(0);
    }
    m_mesh.reset();
}

void Renderer::renderCornerDebugOverlay(RenderParams params)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE;
    window_pos.y = (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE;
    window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

    if (ImGui::Begin("Debug Overlay", nullptr, window_flags)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ImGui::Text("FPS: %.1f", m_frameRateCalculator.getFrameRate());
        ImGui::Separator();
        ImGui::Text("Coordinates:");

        ImGui::Text("Chunk X: %i", params.chunkCoords.x);
        ImGui::Text("Chunk Z: %i", params.chunkCoords.y);
        ImGui::Text("Global X: %.2f", params.position.x);
        ImGui::Text("Global Y: %.2f", params.position.y);
        ImGui::Text("Global Z: %.2f", params.position.z);

        ImGui::Text("Velocity X: %.2f", params.velocity.x);
        ImGui::Text("Velocity Y: %.2f", params.velocity.y);
        ImGui::Text("Velocity Z: %.2f", params.velocity.z);

        ImGui::Text("Acceleration X: %.2f", params.acceleration.x);
        ImGui::Text("Acceleration Y: %.2f", params.acceleration.y);
        ImGui::Text("Acceleration Z: %.2f", params.acceleration.z);

        GlobalParamsLogger::displayDebugParams();
    }
    ImGui::End();
}

void Renderer::displayChunkOverlay(RenderParams params)
{
    const int RADIUS = m_renderDistance + 3; // How many chunks to show in each direction
    const float FIXED_OVERLAY_SIZE = 300.0f; // Fixed size for the overlay window
    // Calculate tile size based on the overlay size and radius
    const float TILE_SIZE = FIXED_OVERLAY_SIZE / (RADIUS * 2 + 7);

    // Position in top-right corner
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - FIXED_OVERLAY_SIZE - 10, 10),
        ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(FIXED_OVERLAY_SIZE, FIXED_OVERLAY_SIZE));

    ImGui::Begin("Chunk Overlay", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar);

    // Get the drawing context
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    // Draw the chunks
    auto chunkMapData = m_chunkMapHandle->getMapDataAccess();
    auto& chunkStates = chunkMapData->states;
    auto& regionData = chunkMapData->regions;

    for (auto chunk : chunkStates)
    {
        glm::ivec2 chunkPos = chunk.first - params.chunkCoords;

        ImVec2 tilePos = ImVec2(
            canvasPos.x + (chunkPos.x + RADIUS) * TILE_SIZE,
            canvasPos.y + (chunkPos.y + RADIUS) * TILE_SIZE
        );

        auto mapColor = ChunkMap::mapStateColors.find(chunk.second)->second;
        auto color = IM_COL32(mapColor.x, mapColor.y, mapColor.z, mapColor.w);

        // Add a small gap between tiles by reducing the size slightly
        float gap = 1.0f;
        draw_list->AddRectFilled(
            tilePos,
            ImVec2(tilePos.x + TILE_SIZE - gap, tilePos.y + TILE_SIZE - gap),
            color
        );

        auto region = regionData.find(chunk.first);
        if (region == regionData.end())
            continue;

        // Draw adjacency triangles
        ImU32 triangleColor = IM_COL32(200, 150, 255, 200);

        // Adjust triangle size to be proportional to tile size
        float halfTile = TILE_SIZE / 2;

        // North
        if (region->second.getAdj(Directions2DHashed::DIRECTION_BACKWARD) != nullptr) {
            draw_list->AddTriangleFilled(
                ImVec2(tilePos.x, tilePos.y),
                ImVec2(tilePos.x + TILE_SIZE - gap, tilePos.y),
                ImVec2(tilePos.x + halfTile, tilePos.y + halfTile),
                triangleColor
            );
        }
        // South
        if (region->second.getAdj(Directions2DHashed::DIRECTION_FORWARD) != nullptr) {
            draw_list->AddTriangleFilled(
                ImVec2(tilePos.x, tilePos.y + TILE_SIZE - gap),
                ImVec2(tilePos.x + TILE_SIZE - gap, tilePos.y + TILE_SIZE - gap),
                ImVec2(tilePos.x + halfTile, tilePos.y + halfTile),
                triangleColor
            );
        }
        // East
        if (region->second.getAdj(Directions2DHashed::DIRECTION_RIGHT) != nullptr) {
            draw_list->AddTriangleFilled(
                ImVec2(tilePos.x + TILE_SIZE - gap, tilePos.y),
                ImVec2(tilePos.x + TILE_SIZE - gap, tilePos.y + TILE_SIZE - gap),
                ImVec2(tilePos.x + halfTile, tilePos.y + halfTile),
                triangleColor
            );
        }
        // West
        if (region->second.getAdj(Directions2DHashed::DIRECTION_LEFT) != nullptr) {
            draw_list->AddTriangleFilled(
                ImVec2(tilePos.x, tilePos.y),
                ImVec2(tilePos.x, tilePos.y + TILE_SIZE - gap),
                ImVec2(tilePos.x + halfTile, tilePos.y + halfTile),
                triangleColor
            );
        }
    }

    ImGui::End();
}

void Renderer::update(RenderParams params)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_renderParams = params;
}

void Renderer::resetViewport(int newWidth, int newHeight)
{
    m_newWidth = newWidth;
    m_newHeight = newHeight;
    m_shouldResetViewport.store(1);
}

float Renderer::updateFrameRate()
{
    return m_frameRateCalculator.updateFrameRate();
}

float Renderer::getFrameRate()
{
    return m_frameRateCalculator.getFrameRate();
}
