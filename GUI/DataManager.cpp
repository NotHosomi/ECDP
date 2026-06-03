#include "DataManager.h"
#include "Config.h"
#include <cassert>

DataManager::DataManager()
{
	Load(Config::Get().ProcessedDataPath());
}

DataManager& DataManager::Get()
{
	static DataManager instance;
	return instance;
}

const Device& DataManager::GetDevice(DeviceId id) const
{
	return m_mDevices.at(id);
}

const T_Design& DataManager::GetDesign(DesignId id) const
{
	return m_mDesigns.at(id);
}

void DataManager::EditDevice(DeviceId id, BatchId batch, DesignId design)
{
	assert(false && "Not yet implemented");
}

bool DataManager::AddDesign(T_Design tDesign)
{
	if (m_mDesigns.find(tDesign.sName) != m_mDesigns.end())
	{
		return false;
	}
	m_mDesigns.insert({ tDesign.sName, tDesign });
	return true;
}

bool DataManager::EditDesign(T_Design tDesign)
{
	if (m_mDesigns.find(tDesign.sName) == m_mDesigns.end())
	{
		return false;
	}
	m_mDesigns[tDesign.sName] = tDesign;
	return true;
}

void DataManager::Load(const std::string& sDirectory)
{
	// todo:
}
