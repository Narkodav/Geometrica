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
	std::string m_hitboxName;
	const Phys::Hitbox* m_hitboxHandle = nullptr; //no hitbox if nullptr

protected:

	// Protected method for derived classes to set properties
	void setId(uint32_t newId) { m_id = newId; }
	void setName(const std::string& newName) { m_name = newName; }
	void setHitbox(const Phys::Hitbox* newHitbox) { m_hitboxHandle = newHitbox; }
	void setHitboxName(const std::string& newHitboxName) { m_hitboxName = newHitboxName; }
	bool loadHitboxData(const std::string& hitboxName);
public:
	virtual ~BlockTemplate() = default;
	uint32_t getId() const { return m_id; };
	std::string getName() const { return m_name; };
	virtual std::string getType() const = 0;
	virtual BlockMesherType getMesherType() const = 0;
	const Phys::Hitbox* getHitbox() const { return m_hitboxHandle; };
	const std::string getHitboxName() const { return m_hitboxName; };
	static std::string parseName(std::string filepath);
	static std::string parseBlockType(const std::string& filepath);
	virtual bool loadFromFile(const std::string& filepath) = 0;
	virtual bool isDynamic() { return false; };
};