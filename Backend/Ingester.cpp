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
#include "Options.h"
#include "Term.h"

# define M_PI           3.14159265358979323846

Ingester::Ingester(std::filesystem::path deviceDirectory)
{
	int verbosity = Options::Get().GetOpt<int>("ingest-verbosity");
	if (verbosity > 0)
	{
		Term::Get()->Println("Files found...");
	}
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
				if (verbosity == 2)
				{
					colcmd = TERM_MAGENTA;
				}
				else
				{
					continue;
				}
			}
			else if (path.find("EIS") != std::string::npos)
			{
				Term::Get()->Colour(Term::E_Colour::RedBold);
				m_vEisPaths.push_back(path);
			}
			else if (path.find("CV") != std::string::npos)
			{
				Term::Get()->Colour(Term::E_Colour::BlueBold);
				m_vCvPaths.push_back(path);
			}
			else if (path.find("CIL") != std::string::npos || path.find("VoltageTransients") != std::string::npos)
			{
				Term::Get()->Colour(Term::E_Colour::YellowBold);
				m_vCilPaths.push_back(path);
			}
			else if (path.find("Crosstalk") != std::string::npos || path.find("Crossimpedance") != std::string::npos)
			{
				Term::Get()->Colour(Term::E_Colour::Cyan);
				m_vCrosstalkPaths.push_back(path);
			}
			else
			{
				Term::Get()->Colour(Term::E_Colour::WhiteBold);
			}
			path.erase(0, deviceDirectory.string().length());
			if (verbosity > 0)
			{
				Term::Get()->Println(" - " + path);
			}
		}
		Term::Get()->Colour(Term::E_Colour::Reset);
	}

	if (!LoadJson<T_DeviceInfo>(deviceDirectory / "DeviceInfo.json", m_tDeviceInfo))
	{
		Term::Get()->Println("Could not read device details");

		Term::Get()->Print("Electrode diameter (microns): ");
		Term::Get()->Read(m_tDeviceInfo.electrodeDiameter);

		Term::Get()->Print("Electrode count: ");
		Term::Get()->Read(m_tDeviceInfo.electrodeCount);

		Term::Get()->Print("Storing device details...");
		if (!SaveJson(deviceDirectory / "DeviceInfo.json", m_tDeviceInfo))
		{
			Term::Get()->Println("Failed to save device details");
		}
		Term::Get()->Println("Done");
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

T_CvElectrodeData Ingester::parseCvFile(const CsvFile& csv) const
{
	try
	{
		T_CvElectrodeData tOutput;
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
		Term::Get()->Println("Failed to parse CV file \"" + csv.GetFilename() + "\"", Term::E_Colour::Red);
		Term::Get()->Colour(Term::E_Colour::Reset);
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

T_EisData Ingester::ParseEis(const std::vector<std::string>& vKeyVals) const
{
	Term::Get()->Print("\nReading EIS file...");
	T_EisData out;
	out.vFrequencies = vKeyVals;

	std::vector<CsvFile> csvList = readFiles(m_vEisPaths);
	if (csvList.size() == 0)
	{
		Term::Get()->Println("Failed - No EIS files");
		return {};
	}

	std::vector<std::vector<double>> allKeyvals(vKeyVals.size(), {});
	for (const auto& entry : csvList)
	{
		// grab raw data
		out.mRaw.insert({ entry.GetFilename(), {} });
		auto& raw = out.mRaw.at(entry.GetFilename());
		raw.vFrequencies = entry.GetColAsDouble("Frequency (Hz)");
		raw.vImpedances = entry.GetColAsDouble("Z (\xCE\xA9)");
		raw.vPhases = entry.GetColAsDouble("-Phase (\xC2\xB0)");

		// grab key impedance values
		std::vector<double> keyvals(vKeyVals.size(), -1);
		for (int row=0; row < entry.GetCol(0).size(); ++row)
		{
			for (int i = 0; i < vKeyVals.size(); ++i)
			{
				if (entry.GetCol("Frequency (Hz)")[row] == vKeyVals[i])
				{
					double val = std::atof(entry.GetCol("Z (\xCE\xA9)")[row].c_str());
					keyvals[i] = val;
					allKeyvals[i].push_back(val);
				}
			}
		}
		out.mImpedances.insert({ entry.GetFilename(), keyvals });
	}
	// todo: add threshold exclusion to the mean calculation here
	std::vector<T_Stats> stats(vKeyVals.size(), {});
	for (int i = 0; i < allKeyvals.size(); ++i)
	{
		T_Stats stats = stddev(allKeyvals[i]);
		out.vAverages.push_back(stats.mean);
		out.vStddev.push_back(stats.stddev);
	}
	Term::Get()->Println("Done");
	return out;
}

T_CvData Ingester::CalculateCscVals() const
{
	T_CvData tOutput;
	std::map<std::string, T_CvElectrodeData> mOutput;
	Term::Get()->Print("Reading CV...");
	std::vector<CsvFile> csvList = readFiles(m_vCvPaths);
	Term::Get()->Println("Done");

	Term::Get()->Print("Calculating CSCs...");
	for(auto entry : csvList)
	{
		T_CvElectrodeData tElecData = parseCvFile(entry);
		if (tElecData.vLoops.size() == 0) { continue; }

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
		for (int i = 1; i < tElecData.vLoops.size(); ++i)
		{
			area += hysteresisArea(tElecData.vLoops[i].vVoltages, tElecData.vLoops[i].vCurrents);
		}
		area /= tElecData.vLoops.size();

		// convert area to CSC
		int windowSize = static_cast<int>((vLoopOffsets[2] - vLoopOffsets[1]) * 0.1f);
		int t1index = vLoopOffsets[1] + windowSize;
		int t0index = vLoopOffsets[1];
		double timeDelta = std::atof(entry.GetCol("Time (s)")[t1index].c_str())
						 - std::atof(entry.GetCol("Time (s)")[t0index].c_str());
		double voltDelta = std::atof(entry.GetCol("WE(1).Potential (V)")[t1index].c_str())
						 - std::atof(entry.GetCol("WE(1).Potential (V)")[t0index].c_str());
		double scanRate = std::abs(voltDelta / timeDelta);
		tElecData.dCsc = (area / (2 * scanRate)) * 1000; // C -> mC
		tElecData.dCscNorm = tElecData.dCsc / GetElectrodeArea_cm2(); // C -> mC
		tOutput.mElectrodes.insert({ entry.GetFilename(), tElecData });
	}
	std::vector<double> vCscs;
	std::vector<double> vCscNorms;
	for (const auto& [key, val] : tOutput.mElectrodes)
	{
		vCscs.push_back(val.dCsc);
		vCscNorms.push_back(val.dCscNorm);
	}
	tOutput.tCsc = stddev(vCscs);
	tOutput.tCscNorm = stddev(vCscNorms);
	Term::Get()->Println("Done");
	return tOutput;
}

T_CilData Ingester::CalculateCilVals() const
{
	if (m_vCilPaths.size() == 0)
	{
		Term::Get()->Println("\nNo voltage transients data found");
		return {};
	}
	Term::Get()->Println("\nReading voltage transients...");
	T_CilData output;
	if (m_vCilPaths.size() > 1)
	{
		Term::Get()->Print("Multiple voltage transient files found. Using " + m_vCilPaths[0].filename().string());
	}
	CsvFile csv(m_vCilPaths[0].string());
	if (csv.GetHeadings().size() <= 1)
	{
		csv = CsvFile(m_vCilPaths[0].string(), ';');
		if (csv.GetHeadings().size() <= 1)
		{
			Term::Get()->Print("Could not read voltage transients file");
			return {}; // todo, try other VT files before giving up
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

const T_DeviceInfo& Ingester::GetDeviceInfo() const
{
	return m_tDeviceInfo;
}
