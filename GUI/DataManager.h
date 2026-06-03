#pragma once
#include <string>
#include <unordered_map>
#include "Device.h"
#include "Design.h"

class DataManager
{
	DataManager();
public:
	static DataManager& Get();

	[[nodiscard]] Device& GetDevice(DeviceId id) const;
	[[nodiscard]] T_Design& GetDesign(DesignId id) const;

	Device* AddDevice(DeviceId id, BatchId batch, DesignId design);
	void EditDevice(DeviceId id, BatchId batch, DesignId design);

	bool AddDesign(T_Design tDesign);
	bool EditDesign(T_Design tDesign);

private:
	void Load(const std::string& sDirectory);

	std::unordered_map<DeviceId, Device> m_mDevices;
	std::unordered_map<DesignId, T_Design> m_mDesigns;
};

