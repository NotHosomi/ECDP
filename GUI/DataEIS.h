#pragma once
#include <map>
#include <string>
#include <vector>
#include "DeviceDetails.h"

struct T_EisPoint
{
	float fHz;
	float fZ;
	float fPhase;
};
using EisSweep = std::vector<T_EisPoint>;
struct T_EisElectrode
{
	unsigned int uElectrode;
	EisSweep vSweep;
};
struct T_EisGross
{
	T_EisGross() = default;
	T_EisGross(DeviceId device, const std::string& sDirectory);

	EisSweep vMean;
	EisSweep vStddev;
	std::vector<T_EisElectrode> vElectrodeData;
};