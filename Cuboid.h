#pragma once
#include <vector>
#include <array>
#include <atomic>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"

#include "platformCommon.h"
#include "Multithreading/Synchronized.h"
//#include "DataManagement/DataRepository.h"

class Cuboid;

struct FaceCombinationsKey
{
    size_t cached_hash;  // Pre-computed hash for instant lookup
    //int smaller_id;      // Sorted IDs for fast comparison
    //int larger_id;
    //FaceRotations normalized_rotation;

    // Constructor can be as "expensive" as needed - it's rarely called
    // if there is no other cuboid, id is -1
    FaceCombinationsKey(const int id1, const int id2, FaceRotations dir) {

        //// Sort IDs for consistent lookup
        //if (id1 <= id2) {
        //    smaller_id = id1;
        //    larger_id = id2;
        //    normalized_rotation = dir;
        //}
        //else {
        //    smaller_id = id2;
        //    larger_id = id1;
        //    normalized_rotation = static_cast<FaceRotations>(getOpposite3D(
        //        static_cast<Directions3DHashed>(dir)));
        //}

        //// Pre-compute hash once and store it
        //cached_hash = compute_hash();

        cached_hash = static_cast<size_t>(id1);
        cached_hash = cached_hash * 31 + static_cast<size_t>(id2);
        cached_hash = cached_hash * 31 + static_cast<size_t>(dir);
    }

    // Equality comparison used during lookup - must be as fast as possible
    [[nodiscard]] bool operator==(const FaceCombinationsKey& other) const noexcept {
        return cached_hash == other.cached_hash;
    }

//private:
//    // Only called once during construction - can be complex if needed
//    [[nodiscard]] size_t compute_hash() const noexcept {
//        size_t hash = static_cast<size_t>(smaller_id);
//        hash = hash * 31 + static_cast<size_t>(larger_id);
//        hash = hash * 31 + static_cast<size_t>(normalized_rotation);
//        return hash;
//    }
};

// Hash function just returns pre-computed value - fastest possible lookup
namespace std {
    template<>
    struct hash<FaceCombinationsKey> {
        [[nodiscard]] size_t operator()(const FaceCombinationsKey& k) const noexcept {
            return k.cached_hash;
        }
    };
}


struct FaceDrawData //for a single cuboid face
{
    unsigned int atlasIndex;
    unsigned char rotationIndex; //matches with FaceRotation enum
    glm::vec3 dimensions;
    glm::vec3 position; //for storing transpositions
};

struct CuboidData
{
    glm::vec3 dimensions;
    glm::vec3 position;

    //returns true if face should be culled
    bool cullFace(const Cuboid& other, const FaceRotations& face,
    const glm::vec3& otherPosition) const; //assumes this is in the center of local coords
};

//NOT THREAD SAFE!!!

//AxisAlligned
class Cuboid
{
public:
	enum class Buffers {
		BUFFER_POSITION,
		BUFFER_INDEX,
		NUM_BUFFERS
	};

    enum class Locations {
        LOCATION_POSITION = 0, //face vertex positions
        LOCATION_TRANSPOSITION = 1, //global world position, added to the vertex positions
        LOCATION_ATLAS_INDEX = 2,
        LOCATION_FACE_INDEX = 3,
        LOCATION_DIMENSIONS = 4,
        NUM_LOCATIONS
    };

    // Positions front (X+) face
    static inline const std::array<glm::vec3, 4> positions = {
        glm::vec3(0.5f, -0.5f,  0.5f),      // vertex 0: bottom-left
        glm::vec3(0.5f, -0.5f, -0.5f),      // vertex 1: bottom-right
        glm::vec3(0.5f,  0.5f, -0.5f),      // vertex 2: top-right
        glm::vec3(0.5f,  0.5f,  0.5f)       // vertex 3: top-left
    };

    // Two triangles (faces), each with 3 indices
    static inline const std::array<glm::uvec3, 2> indices = {
        glm::uvec3(0, 1, 2),    // first triangle
        glm::uvec3(2, 3, 0)     // second triangle
    };

private:

	static inline std::array<unsigned int, static_cast<size_t>(Buffers::NUM_BUFFERS)> buffers;
    static inline unsigned int vao;
	static inline size_t instanceCount = 0; //number of created cuboid class objects
    static inline size_t loadedCount = 0; //number of unique loaded data
    static inline MT::Synchronized<
    std::unordered_map<FaceCombinationsKey, std::vector<FaceDrawData>>> faceCombinationsCache;

    std::vector<CuboidData> m_cuboidData;
    std::string m_name;
    int m_dataId = -1;
public:
	Cuboid();
	/*Cuboid(std::vector<CuboidData> data);*/
	~Cuboid();

    Cuboid(const Cuboid& other)
        : m_cuboidData(other.m_cuboidData)
        , m_dataId(other.m_dataId)
    {
        instanceCount++;
    }

    // Copy assignment operator
    Cuboid& operator=(const Cuboid& other) {
        m_cuboidData = other.m_cuboidData;
        m_dataId = other.m_dataId;
        return *this;
    }

    // Move constructor
    Cuboid(Cuboid&& other) noexcept
        : m_cuboidData(other.m_cuboidData)
        , m_dataId(other.m_dataId)
    {
        instanceCount++;
    }

    // Move assignment operator
    Cuboid& operator=(Cuboid&& other) noexcept {
        m_cuboidData = other.m_cuboidData;
        m_dataId = other.m_dataId;
        return *this;
    }

    bool operator==(const Cuboid& other) const
    {
        return m_dataId == other.m_dataId;
    }

    const std::vector<CuboidData>& getCuboidData() const { return m_cuboidData; };

    static unsigned int getVao() { return vao; };
    static unsigned int getBuffer(Buffers buffer) { return buffers[static_cast<size_t>(buffer)]; };
    static unsigned int getNumberOfIndices() { return indices.size() * 3; };
    
    std::vector<FaceDrawData> getFaceDrawData(const Cuboid& adjacentCuboid,
        const glm::vec3& adjacentPosition,
        FaceRotations rotation, unsigned int atlasIndex) const;

    std::vector<FaceDrawData> getFaceDrawData( 
    FaceRotations rotation, unsigned int atlasIndex) const;

    unsigned int getDataId() const { return m_dataId; };

    void drawTest(std::vector<glm::vec3> positions, std::vector<unsigned int> atlasIndices) const;

    void load(const std::string& filepath);

private:
    void makeBuffers();
    void deleteBuffers();

};

std::string getName(std::string filepath);