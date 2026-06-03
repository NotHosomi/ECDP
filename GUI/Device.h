#pragma once
#include <optional>
#include "DeviceDetails.h"
#include "DataEIS.h"
#include "DataCV.h"

class Device
{
public:
	Device(T_DeviceDetails tDetails);
	Device(const std::string sFilename);

	const T_DeviceDetails& GetDetails() const;
	const std::vector<float>& GetImpedences() const;
	const T_EisGross* GetEIS() const;
	const T_CvGross* GetCV() const;

	void IngestImpedences();
	void IngestEIS();
	void IngestCV();
	void IngestCIL();
private:
	T_DeviceDetails m_tDeviceDetails;
	std::vector<float> m_vImpedences;
	std::optional<T_EisGross> m_oEis;
	std::optional<T_CvGross> m_oCv;

};

