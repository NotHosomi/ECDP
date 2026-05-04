#include "Ingester.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <map>
#include <algorithm>
#include "TerminalColours.h"
#include "CsvFile.h"
#include "stddev.h"
#include "CvData.h"
#include "JsonLoader.h"
#include "stddev.h"

# define M_PI           3.14159265358979323846

Ingester::Ingester(std::filesystem::path deviceDirectory)
{
	std::cout << "Files found..." << std::endl;
	std::string path;
	std::string colcmd;
	std::string printPath;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(deviceDirectory))
	{
		if (entry.is_regular_file())
		{
			path = entry.path().string();
			colcmd = TERM_RESET;
			if (path.find("exclude") != std::string::npos || path.find(".txt") == std::string::npos || path.find(".png") != std::string::npos)
			{
				colcmd = TERM_MAGENTA;
			}
			else if (path.find("EIS") != std::string::npos)
			{
				colcmd = TERM_BOLDRED;
				m_vEisPaths.push_back(path);
			}
			else if (path.find("CV") != std::string::npos)
			{
				colcmd = TERM_BOLDBLUE;
				m_vCvPaths.push_back(path);
			}
			else if (path.find("CIL") != std::string::npos || path.find("VoltageTransients") != std::string::npos)
			{
				colcmd = TERM_BOLDYELLOW;
				m_vCilPaths.push_back(path);
			}
			else if (path.find("Crosstalk") != std::string::npos || path.find("Crossimpedance") != std::string::npos)
			{
				colcmd = TERM_CYAN;
				m_vCrosstalkPaths.push_back(path);
			}
			else
			{
				colcmd = TERM_BOLDWHITE;
			}
			path.erase(0, deviceDirectory.string().length());
			std::cout << TERM_RESET << " - " << colcmd << path << std::endl;
		}
		std::cout << TERM_RESET;
	}

	if (!LoadJson<T_DeviceInfo>(deviceDirectory / "DeviceInfo.json", m_tDeviceInfo))
	{
		std::cout << "Could not read device details" << std::endl;
		std::cout << "Electrode diameter (microns): ";
		while (!(std::cin >> m_tDeviceInfo.electrodeDiameter))
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		std::cout << "Electrode count (microns): ";
		while (!(std::cin >> m_tDeviceInfo.electrodeCount))
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		std::cout << "Storing device details..." << std::flush;
		if (!SaveJson(deviceDirectory / "DeviceInfo.json", m_tDeviceInfo))
		{
			std::cout << "Failed to save device details" << std::endl;
		}
		std::cout << "Done" << std::endl;
	}
}

std::vector<CsvFile> Ingester::readFiles(const std::vector<std::filesystem::path>& fileaddrs) const
{
	std::vector<CsvFile> csvList;
	for (auto& path : fileaddrs)
	{
		csvList.emplace_back(path.string(), ';');
		if (csvList.back().GetHeadings().size() == 1) // in case it has a different delimiter
		{
			csvList.back() = CsvFile(path.string(), ',');
		}
	}
	return csvList;
}

T_CvData Ingester::parseCvFile(const CsvFile& csv) const
{
	try
	{
		T_CvData tOutput;
		std::vector<std::string> scanStrs = csv.GetCol("Scan");
		std::vector<std::string> voltageStrs = csv.GetCol("WE(1).Potential (V)");
		std::vector<std::string> currentStrs = csv.GetCol("WE(1).Current (A)");

		std::vector<int> vLoopOffsets;
		for (int i = 0; i < csv.GetCol(0).size(); ++i)
		{
			int loop = std::atoi(scanStrs[i].c_str());
			if (loop != tOutput.vLoops.size())
			{
				tOutput.vLoops.emplace_back();
				tOutput.vLoops.back().nLoopIndex = static_cast<int>(tOutput.vLoops.size()) - 1;
			}
			tOutput.vLoops.back().vCurrents.push_back(std::atof(currentStrs[i].c_str()));
			tOutput.vLoops.back().vVoltages.push_back(std::atof(voltageStrs[i].c_str()));
		}

		return tOutput;
	}
	catch (std::exception e)
	{
		std::cout << TERM_RED << "Failed to parse CV file \"" << csv.GetFilename() << "\"" << TERM_RESET << std::endl;
		return {};
	}
}

double Ingester::hysteresisArea(const std::vector<double>& x, const std::vector<double>& y) const
{
	double area = 0.0;
	int n = static_cast<int>(x.size());
	int j;
	for (int i = 0; i < n; ++i)
	{
		j = (i + 1) % n;
		area += x[i] * y[j] - x[j] * y[i];
	}
	return std::abs(area) / 2.0;
}

std::map<std::string, std::vector<double>> Ingester::GetEisKeyvals(const std::vector<std::string>& vKeyVals) const
{
	std::map<std::string, std::vector<double>> out;
	std::vector<CsvFile> csvList = readFiles(m_vEisPaths);
	std::vector<std::vector<double>> allKeyvals(vKeyVals.size(), {});
	for (const auto& entry : csvList)
	{
		std::vector<double> keyvals(vKeyVals.size(), -1);
		int index = 0;
		for (; index < entry.GetCol(0).size(); ++index)
		{
			for (int i = 0; i < vKeyVals.size(); ++i)
			{
				if (entry.GetCol("Frequency (Hz)")[index] == vKeyVals[i])
				{
					double val = std::atof(entry.GetCol("Z (\xCE\xA9)")[index].c_str());
					keyvals[i] = val;
					allKeyvals[i].push_back(val);
				}
			}
		}
		out.insert({ entry.GetFilename(), keyvals });
	}
	std::vector<T_Stats> stats(vKeyVals.size(), {});
	std::vector<double> vAvrgs(vKeyVals.size(), 0);
	std::vector<double> vStddev(vKeyVals.size(), 0);
	for (int i = 0; i < allKeyvals.size(); ++i)
	{
		T_Stats stats = stddev(allKeyvals[i]);
		vAvrgs[i] = stats.mean;
		vStddev[i] = stats.stddev;
	}
	out.insert({ "{AVRG}", vAvrgs });
	out.insert({ "{STDDEV}", vStddev });
	return out;
}

// todo: fix csc nan
std::map<std::string, T_CvData> Ingester::CalculateCscVals() const
{
	std::map<std::string, T_CvData> mOutput;
	std::cout << "Reading CV..." << std::endl;
	std::vector<CsvFile> csvList = readFiles(m_vCvPaths);

	std::cout << "Calculating CSCs..." << std::endl;
	for(auto entry : csvList)
	{
		T_CvData tOutput = parseCvFile(entry);
		if (tOutput.vLoops.size() == 0) { continue; }

		std::vector<int> vLoopOffsets;
		vLoopOffsets.push_back(0);
		std::vector<int> scanCol = entry.GetColAsInt("Scan");
		for (int i = 0; i < entry.GetCol(0).size(); ++i)
		{
			int loop = scanCol[i];
			if (loop != vLoopOffsets.size())
			{
				vLoopOffsets.push_back(i);
			}
		}

		double area = 0;
		for (int i = 1; i < tOutput.vLoops.size(); ++i)
		{
			area += hysteresisArea(tOutput.vLoops[i].vVoltages, tOutput.vLoops[i].vCurrents);
		}
		area /= tOutput.vLoops.size();

		// convert area to CSC
		int windowSize = static_cast<int>((vLoopOffsets[2] - vLoopOffsets[1]) * 0.1f);
		int t1index = vLoopOffsets[1] + windowSize;
		int t0index = vLoopOffsets[1];
		double timeDelta = std::atof(entry.GetCol("Time (s)")[t1index].c_str())
						 - std::atof(entry.GetCol("Time (s)")[t0index].c_str());
		double voltDelta = std::atof(entry.GetCol("WE(1).Potential (V)")[t1index].c_str())
						 - std::atof(entry.GetCol("WE(1).Potential (V)")[t0index].c_str());
		double scanRate = std::abs(voltDelta / timeDelta);
		tOutput.dCsc = (area / (2 * scanRate * GetElectrodeArea_cm2())) * 1000; // C -> mC
		mOutput.insert({ entry.GetFilename(), tOutput });
	}
	return mOutput;
}

T_CilData Ingester::CalculateCilVals() const
{
	if (m_vCilPaths.size() == 0)
	{
		std::cout << "\nNo voltage transients data found" << std::endl;
		return {};
	}
	std::cout << "\nReading voltage transients..." << std::endl;
	T_CilData output;
	if (m_vCilPaths.size() > 1)
	{
		std::cout << "Multiple voltage transient files found. Using " + m_vCilPaths[0].filename().string() << std::endl;
	}
	CsvFile csv(m_vCilPaths[0].string());
	if (csv.GetHeadings().size() == 1)
	{
		csv = CsvFile(m_vCilPaths[0].string(), ';');
		if (csv.GetHeadings().size() == 1)
		{
			std::cout << "Could not read voltage transients file" << std::endl;
		}
	}
	for (int i = 1; i < csv.GetHeadings().size(); ++i)
	{
		output.vPulseWidths.push_back(std::atoi(csv.GetHeadings()[i].c_str()));
	}

	std::vector<T_Stats> statsPerElectrode;
	std::vector<T_Stats> statsPerElectrodeNorm;

	for (ElectrodeNum electrode = 0; electrode < csv.GetCol(0).size(); ++electrode)
	{
		int elecNum = std::atoi(csv.GetCol(0)[electrode].c_str());
		output.mCilVals.insert({ elecNum, {} });
		output.mCilValsNormalised.insert({ elecNum, {} });
		for (int col = 0; col < csv.GetHeadings().size() - 1; ++col)
		{
			double cil =
				std::atof(csv.GetCol(col + 1)[electrode].c_str()) * 1e-6 // scale to amps
				* output.vPulseWidths[col] * 1e-6 // scale to seconds
				* 1e3; // scale to milliCoulombs
			output.mCilVals.at(elecNum).push_back(static_cast<float>(cil));
			double cilNorm = cil / GetElectrodeArea_cm2();
			output.mCilValsNormalised.at(elecNum).push_back(static_cast<float>(cilNorm));
		}
	}

	std::vector<T_Stats> statsPerPulseWidth;
	for (int pulseWidthIndex = 0; pulseWidthIndex < output.vPulseWidths.size(); ++pulseWidthIndex)
	{
		std::vector<float> cilsPerPulseWidth;
		std::vector<float> cilsNormPerPulseWidth;
		for (const auto& [key, val] : output.mCilVals)
		{
			cilsPerPulseWidth.push_back(val[pulseWidthIndex]);
		}
		for (const auto& [key, val] : output.mCilValsNormalised)
		{
			cilsNormPerPulseWidth.push_back(val[pulseWidthIndex]);
		}
		output.vCilStats.push_back(stddev(cilsPerPulseWidth));
		output.vCilStatsNormalised.push_back(stddev(cilsNormPerPulseWidth));
	}	
	return output;
}

std::array<T_ErrorBarD, 2> Ingester::GetEisPlot() const
{
	std::cout << "Building EIS plot..." << std::flush;
	std::vector<CsvFile> csvList = readFiles(m_vEisPaths);

	std::erase_if(csvList, [](const CsvFile& data) {
		for (int i = 0; i < data.GetCol(0).size(); ++i)
		{
			if (data.GetCol("Frequency (Hz)")[i] == "1000")
			{
				return std::atof(data.GetCol("Z (\xCE\xA9)")[i].c_str()) >= 20000;
			}
		}
		return true;
		});
	if (csvList.size() == 0) { return { T_ErrorBarD(), T_ErrorBarD() }; }

	T_ErrorBarD PointsZ;
	T_ErrorBarD PointsPhase;
	for (const auto& freq : csvList[0].GetCol("Frequency (Hz)"))
	{
		PointsZ.x.push_back(std::stof(freq));
		PointsPhase.x.push_back(-std::stof(freq));
	}

	for (int rowindex = 0; rowindex < csvList[0].GetCol(0).size(); ++rowindex)
	{
		float avrgZY = 0;
		std::vector<double> rowZVals;
		std::vector<double> rowPhaseVals;
		float avrgPhaseY = 0;
		for (int fileindex = 0; fileindex < csvList.size(); ++fileindex)
		{
			rowZVals.push_back(std::stof(csvList[fileindex].GetCol("Z (\xCE\xA9)")[rowindex]));
			rowPhaseVals.push_back(-std::stof(csvList[fileindex].GetCol("-Phase (\xC2\xB0)")[rowindex]));
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
T_ErrorBarD Ingester::GetCvPlot(const std::vector<std::string>& vExcludes) const
{
	std::cout << "Building CV plot... " << std::flush;
	std::vector<std::map<T_Voltage, double>> grossCv;
	for (const auto& csv : readFiles(m_vCvPaths))
	{
		//if (std::find(vExcludes.begin(), vExcludes.end(), csv.GetFilename()) == vExcludes.end())
		//{
		//	std::cout << "Skipping " << csv.GetFilename() << std::endl;
		//	continue;
		//}
		T_CvData tData = parseCvFile(csv);
		if (tData.vLoops.size() == 0) { continue; }

		// average per loop
		std::map<T_Voltage, double>& loopAvrg = grossCv.emplace_back();	// todo: this doesn't work, cuz it looses the ORDER
		std::map<T_Voltage, int> binInstances;
		for (int i = 1; i < tData.vLoops.size(); ++i)
		{
			const T_CvLoop& loop = tData.vLoops[i];
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
					loopAvrg.insert({ key, loop.vCurrents[j]});
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


	T_ErrorBarD output;
	for (const auto& eDir : { E_ScanDir::Forward, E_ScanDir::Reverse})
	{
		for (const auto& iter : stats)
		{
			if (iter.first.eDir != eDir) { continue; }
			output.x.push_back(iter.first.dVal);
			output.y.push_back(iter.second.mean);
			output.err.push_back(iter.second.stddev);
		}
	}
	// complete the loop
	output.x.push_back(output.x.front());
	output.y.push_back(output.y.front());
	output.err.push_back(output.err.front());
	std::cout << "Done" << std::endl;
	return output;
}

const std::vector<std::filesystem::path> Ingester::GetEisFiles() const
{
	return m_vEisPaths;
}

const std::vector<std::filesystem::path> Ingester::GetCvFiles() const
{
	return m_vCvPaths;
}

const std::vector<std::filesystem::path> Ingester::GetCilPaths() const
{
	return m_vCilPaths;
}

float Ingester::GetElectrodeDiameter() const
{
	return m_tDeviceInfo.electrodeDiameter;
}

double Ingester::GetElectrodeArea_cm2() const
{
	return GetElectrodeArea_um2() / 1e8;
}

double Ingester::GetElectrodeArea_um2() const
{
	return std::pow(m_tDeviceInfo.electrodeDiameter / 2, 2) * M_PI;
}
