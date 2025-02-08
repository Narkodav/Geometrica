#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

#include "platformCommon.h"
#include "Physics/Hitboxes.h"

class BlockTemplate
{
public:
	enum class Properties : uint32_t {
		None = 0,
		Updateable = 1 << 24,
	};
private:
	uint32_t m_id = 0; //precisely 32 bits, uses bit masking for properties, last byte is for type
	std::string m_name;
	std::string m_type;
	BlockMesherType m_mesherType;
protected:
	// Protected constructor for derived classes
	BlockTemplate() = default;

	// Protected method for derived classes to set properties
	void setId(uint32_t newId) { m_id = newId; }
	void setName(const std::string& newName) { m_name = newName; }
	void setType(const std::string& newType) { m_type = newType; }
	void setMesherType(BlockMesherType newType) { m_mesherType = newType; }
public:
	virtual ~BlockTemplate() = default;
	uint32_t getId() const { return m_id; };
	std::string getName() const { return m_name; };
	std::string getType() const { return m_type; };
	BlockMesherType getMesherType() const { return m_mesherType; };
	virtual const Phys::Hitbox* getHitbox() const = 0; //make it return nullptr if no collisions
	static std::string parseName(std::string filepath);
	static std::string parseBlockType(const std::string& filepath);
	virtual bool loadFromFile(const std::string& filepath) = 0;
};

