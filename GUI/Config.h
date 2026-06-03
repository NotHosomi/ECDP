#pragma once
#include <string>

struct T_Limits
{
	float fImpedanceAliveMax = 15000;
	float fImpedanceUnreliableMax = 100000;
};
class Config
{
private:
	Config();
public:
	static const Config& Get();

	const std::string& RawDataPath() const;
	const std::string& ProcessedDataPath() const;

	const T_Limits ImpedenceLimits() const;
};

