#ifndef UTILITIES_H
#define UTILITIES_H
#pragma once

#include <chrono>
#include <map>
#include <string>
#include <array>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

const int constMinYCoord = 0, constWorldHeight = 512, constChunkSize = 16, constChunkSlice = constChunkSize * constChunkSize, constChunkCapacity = constWorldHeight * constChunkSlice;
const float constFreeFallAcceleration = 9.14f;
const float friction = 0.1f;

//XYZ
const std::array<glm::ivec3, 7> constDirectionVectors3D =
{
	glm::ivec3(1,  0,  0),		// front / south
	glm::ivec3(-1,  0,  0),		// back / north
	glm::ivec3(0,  0,  1),		// right / east
	glm::ivec3(0,  0, -1),		// left / west
	glm::ivec3(0,  1,  0),		//up 
	glm::ivec3(0, -1,  0),		//down
	glm::ivec3(0, 0,  0),		//none (for utility)
};

const std::array<glm::ivec2, 4> constDirectionVectors2D =
{
	glm::ivec2(1,  0),		// front / south
	glm::ivec2(-1,  0),		// back / north
	glm::ivec2(0,  1),		// right / east
	glm::ivec2(0,  -1),		// left / west
};

const std::array<glm::ivec2, 4> constDirectionVectors2DHashed =
{
	glm::ivec2( 1,  0),		// front / south
	glm::ivec2( 0,  1),		// right / east
	glm::ivec2(-1,  0),		// back / north
	glm::ivec2( 0, -1),		// left / west
};

const std::array<glm::ivec3, 6> constDirectionVectors3DHashed =
{
	glm::ivec3( 1,  0,  0),		// front / south
	glm::ivec3( 0,  0,  1),		// right / east
	glm::ivec3( 0,  1,  0),		// up 
	glm::ivec3(-1,  0,  0),		// back / north
	glm::ivec3( 0,  0, -1),		// left / west
	glm::ivec3( 0, -1,  0),		// down
};

const std::array<glm::ivec3, 6> constDirectionVectors3DOppositeHashed =
{
	glm::ivec3(-1,  0,  0),		// back / north
	glm::ivec3(0,  0, -1),		// left / west
	glm::ivec3(0, -1,  0),		// down
	glm::ivec3(1,  0,  0),		// front / south
	glm::ivec3(0,  0,  1),		// right / east
	glm::ivec3(0,  1,  0),		// up 
};

enum class Directions3DHashed : unsigned int
{
	DIRECTION_FORWARD = 0,
	DIRECTION_RIGHT,
	DIRECTION_UP,
	DIRECTION_BACKWARD,
	DIRECTION_LEFT,
	DIRECTION_DOWN,
};

const std::array<Directions3DHashed, 6> constDirections3DOppositeHashed =
{
	Directions3DHashed::DIRECTION_BACKWARD,
	Directions3DHashed::DIRECTION_LEFT,
	Directions3DHashed::DIRECTION_DOWN,
	Directions3DHashed::DIRECTION_FORWARD,
	Directions3DHashed::DIRECTION_RIGHT,
	Directions3DHashed::DIRECTION_UP,
};

enum class Directions2DHashed : unsigned int
{
	DIRECTION_FORWARD = 0,
	DIRECTION_RIGHT,
	DIRECTION_BACKWARD,
	DIRECTION_LEFT,
};

enum class Directions : unsigned int
{
	DIRECTION_FORWARD = 0,
	DIRECTION_BACKWARD,
	DIRECTION_RIGHT,
	DIRECTION_LEFT,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_WEST,
	DIRECTION_EAST,
};

enum class FaceRotations : unsigned int
{
	ROTATION_FRONT = 0,
	ROTATION_RIGHT,
	ROTATION_TOP,
	ROTATION_BACK,
	ROTATION_LEFT,
	ROTATION_BOTTOM,
	ROTATION_NONE,
};


const std::array<std::string, 7> cubeSideNames = { "front", "right", "top", "back", "left", "bottom", "none" };

const std::map<std::string, unsigned int> cubeSideNamesMap = {
	{ "front", 0 },	{ "right",  1 }, 
	{ "top",   2 },	{ "back",   3 },
	{ "left",  4 },	{ "bottom", 5 },
	{ "none",  6 }				  };

enum keys
{
	KEY_W = 0,
	KEY_S,
	KEY_A,
	KEY_D,
	KEY_SPACE,
	KEY_SHIFT,
	KEY_ESC,
	KEY_PLAYER_ACTION_FLAG,
	KEY_COUNT,
};

inline unsigned int getOpposite2D(Directions2DHashed dir)
{
	return (static_cast<unsigned int>(dir) + 2) % 4;
};

inline unsigned int getOpposite3D(Directions3DHashed dir)
{
	return (static_cast<unsigned int>(dir) + 3) % 6;
};

inline unsigned int getOpposite2D(unsigned int dir)
{
	return (dir + 2) % 4;
};

inline unsigned int getOpposite3D(unsigned int dir)
{
	return (dir + 3) % 6;
};

struct keyState
{
	bool state; //1 - pressed, 0 - released
	bool isChanged;
};

struct HashVec2 {
	std::size_t operator()(const glm::vec2& v) const {
		return std::hash<float>{}(v.x) ^ (std::hash<float>{}(v.y) << 1);
	}
};

struct HashIvec2 {
	std::size_t operator()(const glm::ivec2& v) const {
		return std::hash<int>{}(v.x) ^ (std::hash<int>{}(v.y) << 1);
	}
};

struct HashVec3 {
	std::size_t operator()(const glm::vec3& v) const {
		return std::hash<float>{}(v.x) ^ (std::hash<float>{}(v.y) << 1) ^ (std::hash<float>{}(v.z) << 2);
	}
};

struct HashIvec3 {
	std::size_t operator()(const glm::ivec3& v) const {
		return std::hash<int>{}(v.x) ^ (std::hash<int>{}(v.y) << 1) ^ (std::hash<int>{}(v.z) << 2);
	}
};

struct ComparatorIvec2
{
	bool operator()(const glm::ivec2& a, const glm::ivec2& b) const
	{
		// First compare lengths
		float lenA = glm::dot(glm::vec2(a), glm::vec2(a));
		float lenB = glm::dot(glm::vec2(b), glm::vec2(a));

		if (lenA != lenB) {
			return lenA < lenB;
		}

		// If lengths are equal, compare coordinates to ensure uniqueness
		// First compare x, then y
		if (a.x != b.x) {
			return a.x < b.x;
		}
		return a.y < b.y;
	}
};

struct Area
{
	glm::ivec3 corner; //negative x, y, z corner
	glm::ivec3 sizes;
};

enum class BlockMesherType
{
	MESHING_CUBOID = 0,		//for block types using cuboid models
	MESHING_LIQUID,			//for block types that are liquid derived
	//MESHING_COMPLICATED,	//for block types that use complicated models (non cuboid models like spheres) WARNING! can affect performance
	//MESHING_TRANSPARENT,	//for transparent block types like glass
	//MESHING_TRANSLUCENT,	//for translucent block types like tinted glass
	MESHING_NUM				//number of meshing techniques
};



#endif