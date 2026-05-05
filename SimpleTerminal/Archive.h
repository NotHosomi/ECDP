#pragma once
#include <optional>
#include <nlohmann\json.hpp>
#include "DeviceInfo.h"
#include "EisData.h"
#include "CilData.h"
#include "CvData.h"

struct T_DeviceData
{
	std::string sDeviceId;
	T_DeviceInfo tInfo;
	std::optional<T_EisData> tEis;
	std::optional<T_CvData> tCv;
	std::optional<T_CilData> tCil;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(T_DeviceData, sDeviceId, tInfo, tEis, tCv, tCil);
};

class Archive
{
public:
	Archive();
	
	void AddDevice(const T_DeviceData& tDevice);

	void SaveAll();
	void LoadAll();
	void Clear();

	const T_DeviceData& GetDevice(const std::string& sDeviceId);

private:
	std::filesystem::path m_Path = "./Archive";
	bool SaveDevice(std::string sDeviceId);
	bool LoadDevice(std::string sDeviceId);

	bool IsDeviceInCache(std::string sDeviceId);

	std::map<std::string, bool> m_mFreshlyGenerated;
	std::map<std::string, T_DeviceData> m_mDevices;
};

