#pragma once
#include <filesystem>
#include "ErrorBarData.h"
#include <array>

class Ingester
{
public:
	Ingester(std::filesystem::path deviceDirectory);

	std::array<T_ErrorBarF, 2> ReadEISFiles();

private:
	std::vector<std::filesystem::path> m_vEisPaths;
	std::vector<std::filesystem::path> m_vCvPaths;
	std::vector<std::filesystem::path> m_vCilPaths;
};

