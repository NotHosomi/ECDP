#pragma once
#include <filesystem>
#include <array>
#include <map>
#include <utility>
#include "dllspec.h"
#include "Plottables.h"
#include "CsvFile.h"
#include "EisData.h"
#include "CvData.h"
#include "CilData.h"
#include "DeviceInfo.h"


class Ingester
{
public:
	DLL Ingester(std::filesystem::path deviceDirectory);

	DLL T_EisData ParseEis(const std::vector<std::string>& vKeyVals) const;
	DLL T_CvData CalculateCscVals() const;
	DLL T_CilData CalculateCilVals() const;

	DLL std::array<T_ErrorPlotF, 2> GetEisPlot() const;
	DLL T_ErrorPlotF GetCvPlot(const std::vector<std::string>& vExcludes = {}) const;

	DLL const std::vector<std::filesystem::path> GetEisFiles() const;
	DLL const std::vector<std::filesystem::path> GetCvFiles() const;
	DLL const std::vector<std::filesystem::path> GetCilPaths() const;

	DLL float GetElectrodeDiameter() const;
	DLL double GetElectrodeArea_cm2() const;
	DLL double GetElectrodeArea_um2() const;
	DLL const T_DeviceInfo& GetDeviceInfo() const;

private:
	std::vector<CsvFile> readFiles(const std::vector<std::filesystem::path>& fileaddrs) const;
	T_CvElectrodeData parseCvFile(const CsvFile& csv) const;
	double hysteresisArea(const std::vector<double>& x, const std::vector<double>& y) const;
	std::vector<std::filesystem::path> m_vEisPaths;
	std::vector<std::filesystem::path> m_vCvPaths;
	std::vector<std::filesystem::path> m_vCilPaths;
	std::vector<std::filesystem::path> m_vCrosstalkPaths;

	bool FetchDeviceDetails(const std::filesystem::path& path);
	void StoreDeviceDetails(const std::filesystem::path& path, const T_DeviceInfo& info);
	T_DeviceInfo m_tDeviceInfo;
};

