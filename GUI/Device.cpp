#include "Device.h"
#include <cassert>
#include "DataManager.h"
#include "Config.h"
#include <filesystem>

Device::Device(T_DeviceDetails tDetails)
{
	m_tDeviceDetails = tDetails;
}

Device::Device(const std::string sFilename)
{

}

const T_DeviceDetails& Device::GetDetails() const
{
	return m_tDeviceDetails;
}

const std::vector<float>& Device::GetImpedences() const
{
	return m_vImpedences;
}

const T_EisGross* Device::GetEIS() const
{
	if (m_oEis.has_value())
	{
		return &m_oEis.value();
	}
	return nullptr;
}

const T_CvGross* Device::GetCV() const
{
	if (m_oCv.has_value())
	{
		return &m_oCv.value();
	}
	return nullptr;
}

void Device::IngestImpedences()
{
}

void Device::IngestEIS()
{
	m_oEis.emplace(m_tDeviceDetails.sName, Config::Get().RawDataPath() + "/" + m_tDeviceDetails.sName + "/EIS/");
}

void Device::IngestCV()
{
}

void Device::IngestCIL()
{
}
