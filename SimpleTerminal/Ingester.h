#pragma once
#include <filesystem>
#include <array>
#include <map>
#include <utility>
#include "ErrorBarData.h"
#include "CsvFile.h"
#include "CvData.h"

struct T_CilData
{
	std::vector<int> vPulseWidths;
	std::map<int, std::vector<float>> mCilVals;
};

class Ingester
{
public:
	Ingester(std::filesystem::path deviceDirectory);

	std::map<std::string, std::array<double, 3>> GetEisKeyvals();
	std::map<std::string, T_CvData> CalculateCscVals();
	T_CilData CalculateCilVals();

	std::array<T_ErrorBarD, 2> GetEisPlot();
	std::vector<std::pair<double, double>> GetCvPlot(int electrodeIndex = -1);

	const std::vector<std::filesystem::path> GetEisFiles() const;
	const std::vector<std::filesystem::path> GetCvFiles() const;
	const std::vector<std::filesystem::path> GetCilPaths() const;

	float GetElectrodeDiameter();
	double GetElectrodeArea_cm2();
	double GetElectrodeArea_um2();

private:
	std::vector<CsvFile> readFiles(const std::vector<std::filesystem::path>& fileaddrs);
	double hysteresisArea(const std::vector<double>& x, const std::vector<double>& y);
	std::vector<std::filesystem::path> m_vEisPaths;
	std::vector<std::filesystem::path> m_vCvPaths;
	std::vector<std::filesystem::path> m_vCilPaths;

	float m_fElectrodeDiameter;
};

