#pragma once
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "DataManagement/dataRepository.h"
#include "logger/GlobalParamsLogger.h"
#include "Physics/Hitboxes.h"
#include "ChunkManagement/ChunkMap.h"

class PhysicsManager
{
public:
    struct BlockRaycastResult {
        bool hit;
        glm::vec3 hitPosition;
        glm::vec3 hitNormal;
        glm::ivec3 blockPos;
        float distance;
    };

public:
	static glm::vec3 handleBlockCollision(const Phys::Hitbox * entityHitbox, glm::vec3 entityPosition, glm::vec3 delta,
	const Phys::Hitbox* blockHitbox, glm::ivec3 blockPosition);

	static glm::vec3 handlePlayerCollisions(glm::vec3 playerCoords, glm::vec3 delta, const ChunkMap& chunkMap);

    static BlockRaycastResult raycastBlock(Phys::RayCasting::Ray ray, float maxDistance, const ChunkMap& chunkMap);

	static Phys::RayCasting::IntersectResult raycast(Phys::RayCasting::Ray ray, float maxDistance,
    const std::vector<Phys::Hitbox>& hitboxes, const std::vector<glm::vec3>& hitboxPositions);

};

