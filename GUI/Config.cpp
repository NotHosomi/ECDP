#include "Config.h"


Config::Config()
{
	// todo: nlohmann
}

const Config& Config::Get()
{
	static Config instance;
	return instance;
}

const std::string& Config::RawDataPath() const
{
	// temp
	return "/TestData/";
}

const std::string& Config::ProcessedDataPath() const
{
	// temp
	return "/TestDataRaw/";
}

const T_Limits Config::ImpedenceLimits() const
{
	return T_Limits();
}
