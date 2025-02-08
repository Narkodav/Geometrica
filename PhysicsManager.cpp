#include "PhysicsManager.h"

glm::vec3 PhysicsManager::handleBlockCollision(const Phys::Hitbox* entityHitbox, glm::vec3 entityPosition, glm::vec3 delta,
	const Phys::Hitbox* blockHitbox, glm::ivec3 blockPosition)
{
	if (blockHitbox == nullptr)
		return delta;
	if (blockHitbox->intersects(*entityHitbox, blockPosition, entityPosition + glm::vec3(delta.x, 0, 0)))
		delta.x = 0;
	if (blockHitbox->intersects(*entityHitbox, blockPosition, entityPosition + glm::vec3(0, delta.y, 0)))
		delta.y = 0;
	if (blockHitbox->intersects(*entityHitbox, blockPosition, entityPosition + glm::vec3(0, 0, delta.z)))
		delta.z = 0;

	return delta;
}

glm::vec3 PhysicsManager::handlePlayerCollisions(glm::vec3 playerCoords, glm::vec3 delta, const ChunkMap& chunkMap)
{
	glm::ivec3 gridCoords = playerCoords;
	glm::vec3 hitboxCoords = playerCoords;
	hitboxCoords.y += 0.4;
	glm::vec3 modifiedDelta = delta;
	const Phys::Hitbox* entityHitbox = &DataRepository::getHitbox("player");

	for (int y = gridCoords.y - 1; y < gridCoords.y + 3; y++)
		for (int x = gridCoords.x - 1; x < gridCoords.x + 2; x++)
			for (int z = gridCoords.z - 1; z < gridCoords.z + 2; z++)
			{
				modifiedDelta = handleBlockCollision(entityHitbox, hitboxCoords, modifiedDelta,
					DataRepository::getBlock(chunkMap.getBlockId(glm::ivec3(x, y, z)))->getHitbox(),
					glm::ivec3(x, y, z));
			}

	return modifiedDelta;

}

PhysicsManager::BlockRaycastResult PhysicsManager::raycastBlock(Phys::RayCasting::Ray ray, float maxDistance, const ChunkMap& chunkMap)
{
	Phys::RayCasting::Ray rayOffset = ray;
	rayOffset.origin += 0.5;
	Phys::RayCasting::RaycastTiled3dResult intersections = Phys::RayCasting::RaycastTiled3d(rayOffset, maxDistance);

	for (glm::ivec3& blockCoord : intersections.intersectedTiles)
	{
		unsigned int blockId = chunkMap.getBlockId(blockCoord);
		if (blockId == DataRepository::airId)
			continue;
		else
		{
			auto result = DataRepository::getBlock(blockId)->getHitbox()->intersectsRay(ray, blockCoord);
			if (result.intersects)
				return { 1, result.intersectionPoint, result.intersectionNormal,blockCoord, result.distance };
		}
	}
	return { 0 };
}

Phys::RayCasting::IntersectResult PhysicsManager::raycast(Phys::RayCasting::Ray ray, float maxDistance,
	const std::vector<Phys::Hitbox>& hitboxes, const std::vector<glm::vec3>& hitboxPositions)
{
	Phys::RayCasting::IntersectResult buffer;
	Phys::RayCasting::IntersectResult final = { 0 };
    float minDistance = INFINITY;

    for (int i = 0; i < hitboxes.size(); i++)
    {
        buffer = hitboxes[i].intersectsRay(ray, hitboxPositions[i]);
        if (buffer.intersects && buffer.distance < minDistance)
            final = buffer;
    }

    return final;
}