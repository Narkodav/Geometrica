#pragma once
//vendor
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//std
#include <thread>
#include <mutex>

//custom
#include "Rendering/Shader.h"
#include "Rendering/Meshing/WorldMesh.h"
#include "Player.h"
#include "FrameRateCalculator.h"
#include "logger/GlobalParamsLogger.h"
#include "Rendering/Billboard.h"
#include "PhysicsManager.h"

#define DEPTH_TESTING 0x10000000
#define FACE_CULLING 0x01000000
#define BLENDING 0x00100000

class Renderer
{
public:
	enum class RenderFlags
	{
		FLAG_DEPTH_TESTING = 0,
		FLAG_FACE_CULLING,
		FLAG_BLENDING,
		FLAG_NUM,
	};

	enum class Shaders
	{
		SHADER_CUBOID = 0,
		SHADER_LIQUID,
		SHADER_BLOCK_HIGHLIGHT,
		SHADER_SKYBOX,
		SHADER_FLAT_TO_SCREEN,
		SHADER_NUM,
	};

	struct RenderParams
	{
		float alpha;
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec3 acceleration;
		glm::mat3 viewWithoutTranspos;
		glm::mat4 interpolatedView;
		glm::ivec2 chunkCoords;
		PhysicsManager::BlockRaycastResult raycastResult;
		Area selectedArea;
	};

private:
	GLFWwindow* m_windowHandle;
	std::thread m_rendererThread;
	std::mutex m_mutex;
	//ThreadPool m_meshLoaderThreads;

	bool m_flags[static_cast<size_t>(RenderFlags::FLAG_NUM)];
	glm::mat4 m_projection;
	Shader m_shaders[static_cast<size_t>(Shaders::SHADER_NUM)];
	float m_specularIntensity;

	std::optional<WorldMesh> m_mesh;
	GameContext m_gameContext;
	const ChunkMap* m_chunkMapHandle;

	int m_renderDistance;
	std::atomic<bool> m_shouldStop = 0;
	bool m_loadingDataRepository = 0;

	std::atomic<bool> m_shouldRender = 0;
	std::atomic<bool> m_shouldResetViewport = 0;
	int m_newWidth = 0;
	int m_newHeight = 0;
	std::atomic<bool> m_isRendering = 0;
	RenderParams m_renderParams;

	FrameRateCalculator m_frameRateCalculator;

	Billboard debugBillboard;

	void rendererLoop();
	void render(RenderParams params);
	void debugRender();

	void setForThread(unsigned int parameters, int renderDistance, float aspectRatio, 
	float fov, GLFWwindow* window, const ChunkMap* chunkMapHandle);
public:

#ifdef _DEBUG
	static void APIENTRY static_glDebugOutput(unsigned int source, unsigned int type, 
	unsigned int id, unsigned int severity,
		int length, const char* message, const void* userParam);
#endif // _DEBUG

	Renderer(const GameContext& gameContext);
	~Renderer();

	void set(unsigned int parameters, int renderDistance, float aspectRatio, 
	float fov, GLFWwindow* window, const ChunkMap* chunkMapHandle);

	void toggleDepthTest();
	void toggleFaceCulling();
	void toggleBlending();
	void setProjection(float aspectRatio, float fov);
	void clear() const;

	void terminate();
	void update(RenderParams params);
	void resetViewport(int newWidth, int newHeight);
	float updateFrameRate();
	float getFrameRate();
	void renderCornerDebugOverlay(RenderParams params);
	void displayChunkOverlay(RenderParams params);

	WorldMesh& getMesh() { return m_mesh.value(); };
};

