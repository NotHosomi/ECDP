#include "Archive.h"
#include <filesystem>
#include "JsonLoader.h"

Archive::Archive()
{
	std::filesystem::create_directory(m_Path);
}

void Archive::AddDevice(const T_DeviceData& tDevice)
{
	m_mDevices[tDevice.sDeviceId] = tDevice;
	m_mFreshlyGenerated[tDevice.sDeviceId] = true;
}

void Archive::SaveAll()
{
	for (const auto& [key, val] : m_mDevices)
	{
		if (m_mFreshlyGenerated.contains(key) && m_mFreshlyGenerated.at(key))
		{
			SaveDevice(key);
			m_mFreshlyGenerated.at(key) = false;
		}
	}
}

void Archive::LoadAll()
{
	// todo: fix this, the .dat extension will break the loading this way
	for (const auto& entry : std::filesystem::directory_iterator(m_Path))
	{
		std::string filename = entry.path().filename().string();
		if (!m_mDevices.contains(filename))
		{
			LoadDevice(filename);
			m_mFreshlyGenerated[filename] = false;
		}
	}
}

void Archive::Clear()
{
	m_mDevices.clear();
	m_mFreshlyGenerated.clear();
}

const T_DeviceData& Archive::GetDevice(const std::string& sDeviceId)
{
	if (m_mDevices.contains(sDeviceId))
	{
		return m_mDevices.at(sDeviceId);
	}
	if (!IsDeviceInCache(sDeviceId))
	{
		if (LoadDevice(sDeviceId))
		{
			return m_mDevices.at(sDeviceId);
		}
	}
	static T_DeviceData null;
	return null;
}

bool Archive::SaveDevice(std::string sDeviceId)
{
	if (!m_mDevices.contains(sDeviceId))
	{
		return false;
	}
	if (m_mFreshlyGenerated.contains(sDeviceId) && !m_mFreshlyGenerated.at(sDeviceId))
	{
		// no point saving a device that was loaded from data anyway
		return true;
	}
	std::string sFilename = sDeviceId + ".dat";
	return SaveBson(m_Path / sFilename, m_mDevices.at(sDeviceId));
}

bool Archive::LoadDevice(std::string sDeviceId)
{
	// is the device data already in memory?
	if (m_mDevices.contains(sDeviceId)) { return false; }

	std::string sFilename = sDeviceId + ".dat";
	return LoadBson(m_Path / sFilename, m_mDevices[sDeviceId]);
}

bool Archive::IsDeviceInCache(std::string sDeviceId)
{
	for (const auto& item : std::filesystem::directory_iterator(m_Path))
	{
		if (item.path().filename() == sDeviceId)
		{
			return true;
		}
	}
	return false;
}
