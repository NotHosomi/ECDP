#pragma once
#include <string>

using DeviceId = std::string;
using BatchId = std::string;
using DesignId = std::string;
struct T_DeviceDetails
{
	DeviceId sName;
	BatchId batch;
	DesignId design;
};