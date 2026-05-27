#include "Core.h"
#include <iostream>
#include <algorithm>
#include "Ingester.h"
#include "TerminalColours.h"
#include "PrintTable.h"
#include "StrUtils.h"
#include "JsonLoader.h"
#include "Commands.h"
#include "Options.h"
#include "GrapherMatplotplusplus.h"
#include "GrapherPy.h"


Core::Core()
{
	std::cout << "Loading user config..." << std::flush;
	if (!LoadJson<T_UserConfig>("./UserConfig.json", m_tUserConfig))
	{
		std::cout << "Failed -- Creating default..." << std::flush;
		SaveJson("./UserConfig.json", m_tUserConfig);
	}
	std::cout << "Done" << std::endl;

	// Validate data dir
	m_DataPath = m_tUserConfig.dataDirectory;
	if (!std::filesystem::exists(m_DataPath))
	{
		do
		{
			if (m_DataPath != "")
			{
				std::cout << TERM_BOLDRED << "Data path \"" + m_DataPath.string() + "\" does not exist" << TERM_RESET << std::endl;
			}
			std::cout << "Input data path: ";
			std::cin >> m_DataPath;
			std::cin.clear();
		} while (!std::filesystem::exists(m_DataPath));
	}
	else
	{
		std::cout << "Data directory: " << std::filesystem::path(m_DataPath) << std::endl;
	}
	std::string plotDir = m_tUserConfig.plotDirectory;
	if (plotDir == "")
	{
		plotDir = "./Plots";
	}
}

E_CmdErr Core::Run(const std::string sDeviceId, E_DataTypes eModes)
{
	if (!LoadGrapher())
	{
		return E_CmdErr::BadOptions;
	}

	std::filesystem::path devicePath = UserConfig().dataDirectory + "/" + sDeviceId;
	if (!std::filesystem::exists(devicePath))
	{
		std::cout << "Could not find " << sDeviceId << std::endl;
		return E_CmdErr::BadArgs;
	}


	std::cout << "\nReading device " << sDeviceId << "\n-------------------" << std::endl;
	Ingester ingest(devicePath);


	// todo: pull from archive
	T_DeviceData data = m_Archive.GetDevice(sDeviceId);
	bool isInArchive = data.sDeviceId != "";
	if (isInArchive)
	{
		std::cout << "Device " << sDeviceId << " found in archive" << std::endl;
	}
	else
	{
		data.sDeviceId = sDeviceId;
		data.tInfo = ingest.GetDeviceInfo();
	}

	// compare
	// See if the archived data is missing components
	// run the missing components
	// add the data to the archive


	if (eModes & E_DataTypes::kEis)
	{
		Eis(data, ingest, m_tUserConfig.eis);
	}
	if (eModes & E_DataTypes::kCv)
	{
		Cv(data, ingest, m_tUserConfig.cv);
	}
	if (eModes & E_DataTypes::kCil)
	{
		Cil(data, ingest, m_tUserConfig.cil);
	}

	if (!isInArchive || Options::Get().GetOpt<bool>("arch-overwrite"))
	{
		m_Archive.AddDevice(data);
	}
	m_Archive.SaveAll();

	std::cout << "\nFinished " << sDeviceId << "\n" << std::endl;
	return E_CmdErr::None;
}

E_CmdErr Core::Plot(const std::string sDeviceId, E_DataTypes eModes)
{
	T_DeviceData data = m_Archive.GetDevice(sDeviceId);
	if (data.sDeviceId == "")
	{
		std::cout << "Device must be ingested before plotting" << std::endl;
	}
	if (eModes & E_DataTypes::kEis)
	{
		if (!data.tEis.has_value())
		{
			std::cout << "Device hasn't ingested EIS" << std::endl;
		}
		else
		{
			PlotEis(data);
		}
	}
	if (eModes & E_DataTypes::kCv)
	{
		if (!data.tCv.has_value())
		{
			std::cout << "Device hasn't ingested CIL" << std::endl;
		}
		else
		{
			PlotCv(data);
		}
	}
	if (eModes & E_DataTypes::kCil)
	{
		if (!data.tCil.has_value())
		{
			std::cout << "Device hasn't ingested CIL" << std::endl;
		}
		else
		{
			PlotCil(data);
		}
	}
	return E_CmdErr::None;
}

T_EisData Core::Eis(T_DeviceData& tDeviceData, const Ingester& ingest, const T_EisConfig& tConfig)
{
	// EIS
	if (!tDeviceData.tEis.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		std::cout << "\nFetching EIS values..." << std::flush;
		T_EisData newdata = ingest.ParseEis(tConfig.keyVals);
		if (newdata.mImpedances.size() != 0)
		{
			tDeviceData.tEis = newdata;
			std::cout << "Done" << std::endl;
		}
	}

	if (!tDeviceData.tEis.has_value())
	{
		return {};
	}
	if (tDeviceData.tEis.value().mImpedances.size() == 0)
	{
		return {};
	}
	// EIS Table
	PrintEisVals(tDeviceData.tEis.value(), tConfig);

	return tDeviceData.tEis.value();
}

T_CvData Core::Cv(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CvConfig& tConfig)
{
	if (!tConfig.calcCsc) { return {}; }


	if (!tDeviceData.tCv.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		T_CvData newdata = ingest.CalculateCscVals();
		if (newdata.mElectrodes.size() == 0)
		{
			std::cout << "No CV data found" << std::endl;
		}
		else
		{
			tDeviceData.tCv = newdata;
		}
	}

	if (tDeviceData.tCv.value().mElectrodes.size() == 0)
	{
		return {};
	}	
	//std::vector<std::string> cvExcludes;
	PrintCscVals(tDeviceData.tCv.value());

	PlotCv(tDeviceData);
	return tDeviceData.tCv.value();
}

T_CilData Core::Cil(T_DeviceData& tDeviceData, const Ingester& ingest, const T_CilConfig& tConfig)
{
	if (!tConfig.calcCil)
	{
		return {};
	}

	if (!tDeviceData.tCil.has_value() || Options::Get().GetOpt<bool>("arch-overwrite") || Options::Get().GetOpt<bool>("arch-recalc"))
	{
		T_CilData newdata = ingest.CalculateCilVals();
		
		if (newdata.mCilVals.size() == 0)
		{
			std::cout << "No CIL data found" << std::endl;
		}
		else
		{
			tDeviceData.tCil = newdata;
		}
	}

	if (!tDeviceData.tCil.has_value())
	{
		return {};
	}
	const T_CilData& tCilData = tDeviceData.tCil.value();
	if (tCilData.vPulseWidths.size() == 0)
	{
		return {};
	}
	std::cout << "\nCIL values" << std::endl;
	PrintCilVals(tCilData.vPulseWidths, tCilData.mCilVals, tCilData.vCilStats);
	std::cout << "\nNormalised CIL values" << std::endl;
	PrintCilVals(tCilData.vPulseWidths, tCilData.mCilValsNormalised, tCilData.vCilStatsNormalised);

	if (tConfig.plotCil)
	{
		// Plot CIL values
		//m_pGrapher->CilAverage(tDeviceData.sDeviceId, tCilData);
	}
	if (tConfig.plotEachElectrode)
	{
		m_pGrapher->CilMulti(tDeviceData.sDeviceId, tCilData);
	}
	return tCilData;
}

T_UserConfig& Core::UserConfig()
{
	return m_tUserConfig;
}

void Core::PrintEisVals(const T_EisData& tEisData, const T_EisConfig& tConfig)
{
	std::vector<std::string> headers;
	headers.push_back("Electrode");
	for (auto keyval : tConfig.keyVals)
	{
		headers.push_back(keyval + " Hz");
	}
	PrintTable EisTable(headers);
	std::vector<std::string> eisExclusionList;
	std::vector<std::string> newRow;
	std::string colour;
	for (const auto& iter : tEisData.mImpedances)
	{
		colour = TERM_GREEN;
		if (std::any_of(iter.second.begin(), iter.second.end(), [tConfig](double val) { return val > tConfig.maxValidImpedance; }))
		{
			colour = TERM_RED;
		}
		EisTable.AddRow(iter.first, iter.second, colour);
	}
	colour = TERM_GREEN;
	if (std::any_of(tEisData.vAverages.begin(), tEisData.vAverages.end(), [tConfig](double val) { return val > tConfig.maxValidImpedance; }))
	{
		colour = TERM_RED;
	}
	EisTable.AddRow("AVRG", tEisData.vAverages);
	EisTable.AddRow("STDDEV", tEisData.vStddev);
	EisTable.Print(TERM_BOLDRED);
}

void Core::PrintCscVals(const T_CvData& tCvData)
{
	PrintTable CscTable({ "Electrode", "CSC (mC)", "Normalised (mC/cm^2)" });
	double sum = 0.0;
	for (const auto& iter : tCvData.mElectrodes)
	{
		std::string colour = "";
		if (iter.second.dCscNorm < 1 || iter.second.dCscNorm != iter.second.dCscNorm)
		{
			colour = TERM_RED;
		}
		CscTable.AddRow({iter.first, std::to_string(iter.second.dCsc), std::to_string(iter.second.dCscNorm)}, colour);
		sum += iter.second.dCscNorm;
	}
	CscTable.AddRow("AVRG", std::vector<double>({ tCvData.tCsc.mean, tCvData.tCscNorm.mean }));
	CscTable.AddRow("STDDEV", std::vector<double>({ tCvData.tCsc.stddev, tCvData.tCscNorm.stddev }));
	CscTable.Print(TERM_BOLDBLUE);
}

void Core::PrintCilVals(std::vector<int> vPulseWidths, std::map<int, std::vector<float>> mVals, std::vector<T_Stats> vStats)
{
	std::vector<std::string> cilTableHeaders{ "Electrode #" };
	for (const auto& pulseWidth : vPulseWidths) { cilTableHeaders.push_back(std::to_string(pulseWidth) + "us"); };
	PrintTable cilTable(cilTableHeaders);
	for (const auto& row : mVals)
	{
		std::vector<std::string> rowtext{ std::to_string(row.first) };
		for (const auto& val : row.second) { rowtext.push_back(std::to_string(val)); };
		cilTable.AddRow(rowtext);
	}
	std::vector<std::string> avrgRowText{ "{AVRG}" };
	std::vector<std::string> stddevRowText{ "{STDDEV}" };
	for (const auto& stats : vStats)
	{
		avrgRowText.push_back(std::to_string(stats.mean));
		stddevRowText.push_back(std::to_string(stats.stddev));
	};
	cilTable.AddRow(avrgRowText);
	cilTable.AddRow(stddevRowText);
	cilTable.Print(TERM_YELLOW);
}

void Core::PlotEis(T_DeviceData& tDeviceData)
{
	if (!tDeviceData.tEis.has_value())
	{
		std::cout << "No EIS data" << std::endl;
		return;
	}
	// EIS Plot
	if (Options::Get().GetOpt<bool>("eis-plot-avrg"))
	{
		std::array<T_ErrorPlotF, 2> EisData = BuildEisPlot(tDeviceData.tEis.value());
		m_pGrapher->EisAverage(tDeviceData.sDeviceId, EisData[0], EisData[1]);
	}
	else
	{
		std::cout << "Skipping average EIS plot (eis-plot-avrg=0)" << std::endl;
	}
	if (Options::Get().GetOpt<bool>("eis-plot-each"))
	{
		for (const auto [key, data] : tDeviceData.tEis.value().mRaw)
		{
			m_pGrapher->EisSingle(tDeviceData.sDeviceId, key, data);
		}
	}
	else
	{
		std::cout << "Skipping per-electrode EIS plots (eis-plot-each=0)" << std::endl;
	}
}

void Core::PlotCv(T_DeviceData& tDeviceData)
{
	if (!tDeviceData.tCv.has_value())
	{
		std::cout << "No CV data" << std::endl;
		return;
	}

	// Plot aggregate CV
	if (Options::Get().GetOpt<bool>("cv-plot-avrg"))
	{
		T_ErrorPlotF tCvPlot = BuildCvPlot(tDeviceData.tCv.value());
		m_pGrapher->CvAverage(tDeviceData.sDeviceId, tCvPlot);
	}
	// Plot each CV
	if (Options::Get().GetOpt<bool>("cv-plot-each"))
	{
		for (const auto& [key, data] : tDeviceData.tCv.value().mElectrodes)
		{
			m_pGrapher->CvSingle(tDeviceData.sDeviceId, key, data);
		}
	}
}

void Core::PlotCil(T_DeviceData& tDeviceData)
{
}

std::array<T_ErrorPlotF, 2> Core::BuildEisPlot(const T_EisData& tData)
{
	std::cout << "Building EIS plot..." << std::flush;
	std::vector<std::string> excludeElectrodes;
	for (const auto& [electrode, data] : tData.mRaw)
	{
		for (const auto& impedence : data.vImpedances)
		{
			if (impedence > Options::Get().GetOpt<float>("eis-impedence-limit"));
			{
				excludeElectrodes.push_back(electrode);
				break;
			}
		}
	}
	
	if (excludeElectrodes.size() >= tData.mRaw.size() - 1)
	{
		excludeElectrodes.clear();
	}


	T_ErrorPlotF PointsZ;
	T_ErrorPlotF PointsPhase;
	PointsZ.x.insert(PointsZ.x.end(), tData.mRaw.begin()->second.vFrequencies.begin(), tData.mRaw.begin()->second.vFrequencies.end());
	PointsPhase.x.insert(PointsPhase.x.end(), tData.mRaw.begin()->second.vFrequencies.begin(), tData.mRaw.begin()->second.vFrequencies.end());

	for (int i = 0; i < tData.mRaw.begin()->second.vImpedances.size(); ++i)
	{
		float avrgZY = 0;
		std::vector<double> rowZVals;
		std::vector<double> rowPhaseVals;
		for (const auto& [key, data] : tData.mRaw)
		{
			if (std::find(excludeElectrodes.begin(), excludeElectrodes.end(), key) != excludeElectrodes.end())
			{
				continue;
			}
			rowZVals.push_back(data.vImpedances[i]);
			rowPhaseVals.push_back(data.vPhases[i]);
		}
		T_Stats rowZStats = stddev(rowZVals);
		PointsZ.y.push_back(rowZStats.mean);
		PointsZ.err.push_back(rowZStats.stddev);
		T_Stats rowPhaseStats = stddev(rowPhaseVals);
		PointsPhase.y.push_back(rowPhaseStats.mean);
		PointsPhase.err.push_back(rowPhaseStats.stddev);
	}
	std::cout << " Done" << std::endl;
	return { PointsZ, PointsPhase };
}

enum E_ScanDir
{
	Forward,
	Reverse
};
struct T_Voltage
{
	double dVal;
	E_ScanDir eDir;
	bool operator<(const T_Voltage& other) const
	{
		if (static_cast<int>(eDir) == static_cast<int>(other.eDir))
		{
			if (eDir == E_ScanDir::Forward)
			{
				return dVal < other.dVal;
			}
			else
			{
				return dVal > other.dVal;
			}
		}
		return static_cast<int>(eDir) < static_cast<int>(other.eDir);
	}
};
T_ErrorPlotF Core::BuildCvPlot(const T_CvData& tData)
{
	std::cout << "Building CV plot... " << std::flush;
	std::vector<std::map<T_Voltage, double>> grossCv;
	for (const auto& [electrode, data] : tData.mElectrodes)
	{
		//if (std::find(vExcludes.begin(), vExcludes.end(), csv.GetFilename()) == vExcludes.end())
		//{
		//	std::cout << "Skipping " << csv.GetFilename() << std::endl;
		//	continue;
		//}
		if (data.vLoops.size() == 0) { continue; }

		// average per loop
		std::map<T_Voltage, double>& loopAvrg = grossCv.emplace_back();	// todo: this doesn't work, cuz it looses the ORDER
		std::map<T_Voltage, int> binInstances;
		for (int i = 1; i < data.vLoops.size(); ++i)
		{
			const T_CvLoop& loop = data.vLoops[i];
			for (int j = 0; j < loop.vVoltages.size(); ++j)
			{
				T_Voltage key;
				// round to 0.0X volts
				double roundedVal = std::round(loop.vVoltages[j] * 100) / 100;
				key.dVal = roundedVal;
				if (j == 0)
				{
					key.eDir = E_ScanDir::Forward;
				}
				else
				{
					key.eDir = loop.vVoltages[j] > loop.vVoltages[j - 1] ? E_ScanDir::Forward : E_ScanDir::Reverse;
				}
				if (!loopAvrg.contains(key))
				{
					loopAvrg.insert({ key, loop.vCurrents[j] });
					binInstances.insert({ key, 1 });
				}
				else
				{
					loopAvrg.at(key) += loop.vCurrents[j];
					binInstances.at(key) += 1;
				}
			}
			for (auto& [key, val] : loopAvrg)
			{
				val /= binInstances.at(key);
			}
		}
	}
	//if (grossCv.size() == 0)
	//{
	//	std::cout << "Not enough valid CV data to plot" << std::endl;
	//	return {};
	//}

	// average across files, with stddev
	std::map<T_Voltage, T_Stats> stats = stddev(grossCv);


	T_ErrorPlotF output;
	for (const auto& eDir : { E_ScanDir::Forward, E_ScanDir::Reverse })
	{
		for (const auto& iter : stats)
		{
			if (iter.first.eDir != eDir) { continue; }
			output.x.push_back(static_cast<float>(iter.first.dVal));
			output.y.push_back(static_cast<float>(iter.second.mean));
			output.err.push_back(static_cast<float>(iter.second.stddev));
		}
	}
	// complete the loop
	output.x.push_back(output.x.front());
	output.y.push_back(output.y.front());
	output.err.push_back(output.err.front());
	std::cout << "Done" << std::endl;
	return output;
}

bool Core::LoadGrapher()
{
	if (m_pGrapher != nullptr)
	{
		m_pGrapher.reset();
	}
	std::string backend = Options::Get().GetOpt<std::string>("plotter-backend");
	if (backend == "internal")
	{
		m_pGrapher = std::make_unique<GrapherMatplotplusplus>();
		return true;
	}
	if (backend == "python")
	{
		m_pGrapher = std::make_unique<GrapherPy>();
		return true;
	}
	if (backend == "qt")
	{
		std::cout << "Warning: Qt plotting backend not yet integrated" << std::endl;
		return false;
	}
	else
	{
		std::cout << "Warning: Unknown plotting backend specified" << std::endl;
		return false;
	}


}

bool Core::BatchAverages(const std::vector<std::string> sIds)
{
	std::vector<T_DeviceData> devices;
	for (const auto& sId : sIds)
	{
		if (m_Archive.GetDevice(sId).sDeviceId == "")
		{
			std::cout << "\nDevice " << sId << " not in archive" << std::flush;
			if (Run(sId, static_cast<E_DataTypes>(E_DataTypes::kEis | E_DataTypes::kCv | E_DataTypes::kCil)) == E_CmdErr::None)
			{
				devices.push_back(m_Archive.GetDevice(sId));
			}
		}
		else
		{
			std::cout << "\nFound device " << sId << " in archive" << std::endl;
			devices.push_back(m_Archive.GetDevice(sId));
		}
	}
	if (devices.size() < 2)
	{
		std::cout << "Need more than 2 devices to run a batch stat pool" << std::endl;
		return false;
	}

	// EIS
	std::map<std::string, std::vector<T_StatGroup>> eisStats;
	for (auto& device : devices)
	{
		if (!device.tEis.has_value())
		{
			std::cout << device.sDeviceId << " is missing EIS data" << std::endl;
			continue;
		}
		const T_EisData& eis = device.tEis.value();
		for (int i = 0; i < eis.vFrequencies.size(); ++i)
		{
			T_StatGroup group;
			group.n = static_cast<int>(eis.mImpedances.size());
			group.mean = eis.vAverages[i];
			group.sd = eis.vStddev[i];
			eisStats[eis.vFrequencies[i]].push_back(group);
		}
	}
	std::map<std::string, T_Stats> eisBatchAvrg;
	PrintTable eisTable({ "Frequency (Hz)", "Avrg Impedance", "Standard Deviation" });
	for (const auto& [freq, groupList] : eisStats)
	{
		T_Stats stats = PooledStddev(groupList);
		eisBatchAvrg.insert({ freq, stats });
		eisTable.AddRow({ freq, SU::RoundToStr(stats.mean), SU::RoundToStr(stats.stddev) });
	}
	std::cout << "\nAverage impedances" << std::endl;
	eisTable.Print(TERM_BOLDGREEN);


	// CV
	std::vector<T_StatGroup> cvStats;
	std::vector<T_StatGroup> cvNormStats;
	for (const auto& device : devices)
	{
		if (!device.tCv.has_value())
		{
			std::cout << device.sDeviceId << " is missing CV data" << std::endl;
			continue;
		}
		const T_CvData& cv = device.tCv.value();
		T_StatGroup group;
		group.n = static_cast<int>(cv.mElectrodes.size());
		group.mean = cv.tCsc.mean;
		group.sd = cv.tCsc.stddev;
		cvStats.push_back(group);
		group.mean = cv.tCscNorm.mean;
		group.sd = cv.tCscNorm.stddev;
		cvNormStats.push_back(group);
	}
	T_Stats cvBatchStats = PooledStddev(cvStats);
	T_Stats cvNormBatchStats = PooledStddev(cvNormStats);
	PrintTable cvTable({ " ", "Average", "Stddev" });
	cvTable.AddRow({ "mC", std::to_string(cvBatchStats.mean), std::to_string(cvBatchStats.stddev) });
	cvTable.AddRow({ "mC/cm^2", std::to_string(cvNormBatchStats.mean), std::to_string(cvNormBatchStats.stddev) });
	std::cout << "\nAverage CSCs" << std::endl;
	cvTable.Print(TERM_BOLDBLUE);

	
	// CIL
	std::map<int, std::vector<T_StatGroup>> cilStats;
	std::map<int, std::vector<T_StatGroup>> cilNormStats;
	for (const auto& device : devices)
	{
		if (!device.tCil.has_value())
		{
			std::cout << device.sDeviceId << " is missing EIS data" << std::endl;
			continue;
		}
		const T_CilData& cil = device.tCil.value();
		T_StatGroup group;
		group.n = static_cast<int>(cil.mCilVals.size());
		for (int i = 0; i < cil.vPulseWidths.size(); ++i)
		{
			group.mean = cil.vCilStats[i].mean;
			group.sd = cil.vCilStats[i].stddev;
			cilStats[cil.vPulseWidths[i]].push_back(group);
			group.mean = cil.vCilStatsNormalised[i].mean;
			group.sd = cil.vCilStatsNormalised[i].stddev;
			cilNormStats[cil.vPulseWidths[i]].push_back(group);
		}
	}
	std::map<int, T_Stats> cilBatchAvrg;
	std::map<int, T_Stats> cilNormBatchAvrg;
	PrintTable cilTable({ "Pulse Width (microseconds)", "Avrg CIL (mC)", "Stddev", "Avrg norm CIL (mC/cm^2)", "Stddev" });
	for (const auto& [pulseWidth, groupList] : cilStats)
	{
		T_Stats stats = PooledStddev(groupList);
		cilBatchAvrg.insert({ pulseWidth, stats });
		T_Stats statsNorm = PooledStddev(cilNormStats.at(pulseWidth));
		cilNormBatchAvrg.insert({ pulseWidth, statsNorm});
		cilTable.AddRow({ std::to_string(pulseWidth), std::to_string(stats.mean), std::to_string(stats.stddev), std::to_string(statsNorm.mean), std::to_string(stats.stddev) });
	}
	std::cout << "\nAverage CILs" << std::endl;
	cilTable.Print(TERM_YELLOW);
	return true;
}