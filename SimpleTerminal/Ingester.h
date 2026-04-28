#pragma once
#include <filesystem>
#include <array>
#include <map>
#include <utility>
#include "ErrorBarData.h"
#include "CsvFile.h"
#include "CvData.h"
#include "CilData.h"
#include "DeviceInfo.h"


class Ingester
{
public:
	Ingester(std::filesystem::path deviceDirectory);

	std::map<std::string, std::vector<double>> GetEisKeyvals(const std::vector<std::string>& vKeyVals) const;
	std::map<std::string, T_CvData> CalculateCscVals() const;
	T_CilData CalculateCilVals() const;

	std::array<T_ErrorBarD, 2> GetEisPlot() const;
	T_ErrorBarD GetCvPlot(const std::vector<std::string>& vExcludes) const;

	const std::vector<std::filesystem::path> GetEisFiles() const;
	const std::vector<std::filesystem::path> GetCvFiles() const;
	const std::vector<std::filesystem::path> GetCilPaths() const;

	float GetElectrodeDiameter() const;
	double GetElectrodeArea_cm2() const;
	double GetElectrodeArea_um2() const;

private:
	std::vector<CsvFile> readFiles(const std::vector<std::filesystem::path>& fileaddrs) const;
	T_CvData parseCvFile(const CsvFile& csv) const;
	double hysteresisArea(const std::vector<double>& x, const std::vector<double>& y) const;
	std::vector<std::filesystem::path> m_vEisPaths;
	std::vector<std::filesystem::path> m_vCvPaths;
	std::vector<std::filesystem::path> m_vCilPaths;
	std::vector<std::filesystem::path> m_vCrosstalkPaths;

	bool FetchDeviceDetails(const std::filesystem::path& path);
	void StoreDeviceDetails(const std::filesystem::path& path, const T_DeviceInfo& info);
	T_DeviceInfo m_tDeviceInfo;
};

