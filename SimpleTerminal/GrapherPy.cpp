#include "GrapherPy.h"
#include <stdlib.h>
#include <string>
#include <filesystem>
#include <iostream>
#include "CsvOut.h"
#include "EisData.h"
#include "CvData.h"
#include "CilData.h"
#include "Options.h"

bool GrapherPy::Precheck()
{
	return system(NULL) != 0;
	// use popen to check if python is installed
}

void GrapherPy::EisAverage(const std::string& sName, const T_ErrorPlotF& tZ, const T_ErrorPlotF& tPhase, bool bReplot)
{

	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sName + "/";
	std::filesystem::create_directories(path);
	std::string file = path + sName + " EIS.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering EIS average..." << std::flush;
	std::string plotDataPath = GetTempPath(sName, "EIS");
	{
		// in a local scope to drop the CsvOut once its been exported
		CsvOut transfer({ "x","y","yerr","y2","y2err" });
		for (int i = 0; i < tZ.x.size(); ++i)
		{
			transfer.AddRow(std::vector<float>{ tZ.x[i], tZ.y[i], tZ.err[i], tPhase.y[i], tPhase.err[i] });
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdEis(plotDataPath, file, sName + " EIS",
		{ Options::Get().GetOpt<float>("plotter-eis-ymin"), Options::Get().GetOpt<float>("plotter-eis-ymax") },
		{ -90.0f,0.0f },
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
}

void GrapherPy::EisSingle(const std::string& sId, const std::string& filename, const T_EisRawData& tRaw, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sId + "/EISs/";
	std::filesystem::create_directories(path);
	std::string file = path + filename + ".png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering EIS for " + filename + "..." << std::flush;
	std::string plotDataPath = GetTempPath(sId + "-" + filename, "EIS");
	{
		// in a local scope to drop the CsvOut once its been exported
		CsvOut transfer({ "x","y","y2" });
		for (int i = 0; i < tRaw.vFrequencies.size(); ++i)
		{
			transfer.AddRow(std::vector<double>{ tRaw.vFrequencies[i], tRaw.vImpedances[i], tRaw.vPhases[i] });
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdEis(plotDataPath, file, sId + " EIS",
		{ Options::Get().GetOpt<float>("plotter-eis-ymin"), Options::Get().GetOpt<float>("plotter-eis-ymax") },
		{ -90.0f,0.0f },
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
}

void GrapherPy::CvAverage(const std::string& sName, T_ErrorPlotF tLoop, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sName + "/";
	std::filesystem::create_directories(path);
	std::string file = path + sName + " CV.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering CV average..." << std::flush;
	std::string plotDataPath = GetTempPath(sName, "CV");
	{
		// in a local scope to drop the CsvOut once its been exported
		CsvOut transfer({ "x","y","yerr" });
		for (int i = 0; i < tLoop.x.size(); ++i)
		{
			transfer.AddRow(std::vector<float>{ tLoop.x[i], tLoop.y[i] * 1e6f, tLoop.err[i] * 1e6f});
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdCv(plotDataPath, file, sName + " CV",
		{ Options::Get().GetOpt<float>("plotter-cv-ylim"), Options::Get().GetOpt<float>("plotter-cv-ylim") },
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
}

void GrapherPy::CvSingle(const std::string& sId, const std::string& filename, T_CvElectrodeData tRaw, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sId + "/CVs/";
	std::filesystem::create_directories(path);
	std::string file = path + filename + ".png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering CV for " + filename + "..." << std::flush;
	std::string plotDataPath = GetTempPath(sId + "-" + filename, "CV");
	{
		std::vector<std::string> headers = { "x" };
		for (int i = 1; i <= tRaw.vLoops.size(); ++i)
		{
			headers.push_back("y_" + std::to_string(i));
		}
		CsvOut transfer(headers);
		for (int i = 0; i < tRaw.vLoops.back().vVoltages.size(); ++i)
		{
			std::vector<double> row = { tRaw.vLoops.back().vVoltages[i] };
			for (const auto& loop : tRaw.vLoops)
			{
				row.push_back(loop.vCurrents[i] * 1e6f);
			}
			transfer.AddRow(row);
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdCv(plotDataPath, file, sId + " " + filename + " CV",
		{ Options::Get().GetOpt<float>("plotter-cv-ymin"), Options::Get().GetOpt<float>("plotter-cv-ymax") },
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
}

void GrapherPy::CilAverage(const std::string& sName, const T_ErrorPlotF& tCil, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sName + "/";
	std::filesystem::create_directories(path);
	std::string file = path + sName + " CIL.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering CIL average..." << std::flush;
	std::string plotDataPath = GetTempPath(sName, "CIL");
	{
		// in a local scope to drop the CsvOut once its been exported
		CsvOut transfer({ "x","y","yerr" });
		for (int i = 0; i < tCil.x.size(); ++i)
		{
			transfer.AddRow(std::vector<float>{ tCil.x[i], tCil.y[i], tCil.err[i]});
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdCil(plotDataPath, file, sName + " CIL",
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
}

void GrapherPy::CilMulti(const std::string& sName, const T_CilData& data, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + sName + "/";
	std::filesystem::create_directories(path);
	std::string file = path + sName + " multi CIL.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	std::cout << "Rendering CIL multiline..." << std::flush;
	std::string plotDataPath = GetTempPath(sName + "-multi", "CIL");
	{
		// in a local scope to drop the CsvOut once its been exported
		std::vector<std::string> headers = { "x" };
		for (int i = 1; i <= data.mCilVals.size(); ++i)
		{
			headers.push_back("y_" + std::to_string(i));
		}
		CsvOut transfer(headers);
		for (int i = 0; i < data.vPulseWidths.size(); ++i)
		{
			std::vector<double> row = { static_cast<double>(data.vPulseWidths[i]) };
			for (const auto& [electrode, cils] : data.mCilVals)
			{
				row.push_back(cils[i]);
			}
			transfer.AddRow(row);
		}
		transfer.SaveTo(plotDataPath);
	}

	CmdCil(plotDataPath, file, sName + " CILs",
		Options::Get().GetOpt<int>("plotter-dpi"),
		Options::Get().GetOpt<int>("plotter-fontsize-labels"),
		Options::Get().GetOpt<int>("plotter-fontsize-ticks"),
		Options::Get().GetOpt<int>("plotter-fontsize-title"));
	
}

std::string GrapherPy::GetTempPath(const std::string& sName, const std::string& sMode)
{
	std::filesystem::create_directory("./temp/");
	if (Options::Get().GetOpt<bool>("plotter-hold-temp"))
	{
		return std::filesystem::current_path().string() + "/temp/temp_" + sName + "_" + sMode + ".dat";
	}
	return std::filesystem::current_path().string() + "/temp/temp.dat";
}

void GrapherPy::CmdEis(
	const std::string& sPlotDataPath,
	const std::string& sOutput,
	const std::string& sTitle,
	const std::pair<float, float>& tYLim,
	const std::pair<float, float>& tY2Lim,
	unsigned int uDpi,
	unsigned int uLabelFontsize,
	unsigned int uTickFontsize,
	unsigned int uTitleFontsize)
{
	std::string cmd = "python ";
	cmd += "\"./Scripts/" + Options::Get().GetOpt<std::string>("plotter-pyscript-eis") + "\"";
	cmd += " \"" + sPlotDataPath + "\"";
	cmd += " --output \"" + sOutput + "\"";
	cmd += " --xlabel \"Frequency (Hz)\"";
	cmd += " --ylabel \"Absolute Impedance (\u03A9)\"";
	cmd += " --ylabel2 \"Phase (\u00B0)\"";
	cmd += " --ylim " + std::to_string(tYLim.first) + " " + std::to_string(tYLim.second);
	cmd += " --ylim2 " + std::to_string(tY2Lim.first) + " " + std::to_string(tY2Lim.second);
	cmd += " --xscale log";
	cmd += " --yscale log";
	cmd += " --dpi " + std::to_string(uDpi);
	cmd += " --label-fontsize " + std::to_string(uLabelFontsize);
	cmd += " --tick-fontsize " + std::to_string(uTickFontsize);
	cmd += " --title-fontsize " + std::to_string(uTitleFontsize);
	
	//std::cout << cmd << std::endl;
	// python ./Scripts/Plotter.py <CD>/temp/test.csv -o <CD>/plots/test2.png --xlabel freq --ylabel imp --ylabel2 phase --ylim 1000,100000 --ylim2 -90 0 --label-fontsize 12 --tick-fontsize 20 --title scrungle --title-fontsize 5
	system(cmd.c_str());
	std::cout << "Done" << std::endl;
}

void GrapherPy::CmdCv(
	const std::string& sPlotDataPath,
	const std::string& sOutput,
	const std::string& sTitle,
	const std::pair<float, float>& tYLim,
	unsigned int uDpi,
	unsigned int uLabelFontsize,
	unsigned int uTickFontsize,
	unsigned int uTitleFontsize)
{
	std::string cmd = "python ";
	cmd += "./Scripts/" + Options::Get().GetOpt<std::string>("plotter-pyscript-cv");
	cmd += " \"" + sPlotDataPath + "\"";
	cmd += " --output \"" + sOutput + "\"";
	cmd += " --xlabel \"Voltage (V)\"";
	cmd += " --ylabel \"Current (\u03BCA)\"";
	cmd += " --ylim " + std::to_string(tYLim.first) + " " + std::to_string(tYLim.second);
	cmd += " --dpi " + std::to_string(uDpi);
	cmd += " --label-fontsize " + std::to_string(uLabelFontsize);
	cmd += " --tick-fontsize " + std::to_string(uTickFontsize);
	cmd += " --title-fontsize " + std::to_string(uTitleFontsize);

	//std::cout << cmd << std::endl;
	system(cmd.c_str());
	std::cout << "Done" << std::endl;
}

void GrapherPy::CmdCil(
	const std::string& sPlotDataPath,
	const std::string& sOutput,
	const std::string& sTitle,
	unsigned int uDpi,
	unsigned int uLabelFontsize,
	unsigned int uTickFontsize,
	unsigned int uTitleFontsize)
{
	std::string cmd = "python ";
	cmd += "./Scripts/" + Options::Get().GetOpt<std::string>("plotter-pyscript-cil");
	cmd += " \"" + sPlotDataPath + "\"";
	cmd += " --output \"" + sOutput + "\"";
	cmd += " --xlabel \"Pulse Width (\u03BCS)\"";
	cmd += " --ylabel \"Charge Limit (mC)\"";
	cmd += " --dpi " + std::to_string(uDpi);
	cmd += " --label-fontsize " + std::to_string(uLabelFontsize);
	cmd += " --tick-fontsize " + std::to_string(uTickFontsize);
	cmd += " --title-fontsize " + std::to_string(uTitleFontsize);

	//std::cout << cmd << std::endl;
	system(cmd.c_str());
	std::cout << "Done" << std::endl;
}
