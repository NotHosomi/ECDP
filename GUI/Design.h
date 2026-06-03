#pragma once
#include <string>
#include "DeviceDetails.h"
#include <vector>

struct T_Coord
{
	int x;
	int y;
};
struct T_Electrode
{
	int nPadId;
	T_Coord tPos;
};
struct T_Design
{
	DesignId sName;
	int nElectrodeCount;
	float fElectrodeDiameter;
	float fElectrodePitch;
	std::vector<T_Electrode> vElectrodes;
};

