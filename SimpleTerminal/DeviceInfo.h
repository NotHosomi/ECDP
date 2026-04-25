#pragma once
#include <string>
#include <nlohmann\json.hpp>

struct T_DeviceInfo
{
	int electrodeDiameter;
	int electrodeCount;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_DeviceInfo, electrodeDiameter, electrodeCount);
};
