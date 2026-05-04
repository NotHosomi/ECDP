#define NOMINMAX
#include "Grapher.h"
#include <algorithm>
#include <matplot/matplot.h>
#include "CilData.h"

Grapher::Grapher(std::filesystem::path outputDir)
{
	SetOutputPath(outputDir);
}

void Grapher::SetOutputPath(std::filesystem::path outputDir)
{
	if (!std::filesystem::exists(outputDir))
	{
		std::filesystem::create_directories(outputDir);
	}
}

void Grapher::GraphEIS(std::string sId, T_ErrorBarD tZ, T_ErrorBarD tPhase)
{
	std::cout << "Rendering EIS plot... " << std::flush;
	using namespace matplot;
	auto fig = figure(true);
	hold(false);
	error_bar_handle magnitudePlot = errorbar(tZ.x, tZ.y, tZ.err);
	gca()->x_axis().scale(axis_type::axis_scale::log);
	gca()->y_axis().scale(axis_type::axis_scale::log);
	double yMin = std::min(  1000.0, gca()->ylim()[0] * 0.9);
	double yMax = std::max(100000.0, gca()->ylim()[1] * 1.1);
	gca()->ylim({ yMin, yMax });

	double logMin = std::log10(yMin);
	double logMax = std::log10(yMax);
	double logScaleFactor = (logMax - logMin) / 90.0;
	std::vector<double> scaledPhase, errNeg, errPos, errX;
	for (int i = 0; i < tPhase.y.size(); ++i)
	{
		double logCenter = (tPhase.y[i] + 90) * logScaleFactor + logMin;
		double logUpper = (tPhase.y[i] + tPhase.err[i] + 90) * logScaleFactor + logMin;
		double logLower = (tPhase.y[i] - tPhase.err[i] + 90) * logScaleFactor + logMin;

		double center = std::pow(10, logCenter);
		scaledPhase.push_back(center);
		errPos.push_back(std::pow(10, logUpper) - center);
		errNeg.push_back(center - std::pow(10, logLower));
		errX.push_back(0);
	}

	gca()->y2lim({ yMin, yMax });
	hold(true);
	//error_bar_handle phasePlot = errorbar(tPhase.x, tPhase.y, tPhase.err);
	//phasePlot->use_y2(true);
	error_bar_handle phasePlot = errorbar(tPhase.x, scaledPhase, errNeg, errPos, errX, errX);
	//gca()->y2lim({ 0, gca()->y2lim()[1] });

	// create the y2 axis
	auto dummy = plot({0,0}, {1,0});
	dummy->use_y2(true);
	y2lim({ -90,0 });

	// captioning
	title(sId + " EIS");
	y2label("Phase (\xC2\xB0)");
	ylabel("Absolute Impedance (\xCE\xA9)");
	xlabel("Frequency (Hz)");


	std::string path = m_PlotDir.string() + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	save(path + "EIS.png");
	std::cout << "Done\n" << std::endl;
}

void Grapher::GraphCV(std::string sId, T_ErrorBarD tLoop)
{
	if (tLoop.x.size() == 0)
	{
		std::cout << "Cannot render CV plot" << std::endl;
		return;
	}
	using namespace matplot;
	std::cout << "Rendering CV plot... " << std::flush;

	// make the data loop
	tLoop.x.push_back(tLoop.x.front());
	tLoop.y.push_back(tLoop.y.front());
	tLoop.err.push_back(tLoop.err.front());


	figure(true);
	hold(false);
	error_bar_handle magnitudePlot = errorbar(tLoop.x, tLoop.y, tLoop.err);

	title(sId + " CV");
	ylabel("Current (A)");
	xlabel("Voltage (V)");

	matplot::xlim({ -0.65, 0.85 });
	matplot::ylim({ -2e-5, 2e-5 });

	std::string path = m_PlotDir.string() + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	matplot::gcf()->width(m_nCvWidth);
	matplot::gcf()->height(m_nCvHeight);
	save(path + "CV.png");
	std::cout << "Done\n" << std::endl;
}

void Grapher::GraphCV(std::string sId, std::string filename, T_CvData tElectrode)
{
	std::cout << "Rendering CV plot (" << filename << ")... " << std::flush;

	matplot::figure(true);
	matplot::hold(true);
	for (int i = 0; i < tElectrode.vLoops.size() - 1; ++i)
	{
		tElectrode.vLoops[i].vVoltages.push_back(tElectrode.vLoops[i+1].vVoltages.front());
		tElectrode.vLoops[i].vCurrents.push_back(tElectrode.vLoops[i+1].vCurrents.front());
		matplot::plot(tElectrode.vLoops[i].vVoltages, tElectrode.vLoops[i].vCurrents);
	}

	matplot::title(filename + " CV");
	matplot::ylabel("Current (A)");
	matplot::xlabel("Voltage (V)");

	matplot::xlim({ -0.65, 0.85 });
	matplot::ylim({ -2e-5, 2e-5 });

	std::string path = m_PlotDir.string() + "/" + sId + "/Plots/CV/";
	std::filesystem::create_directories(path);
	matplot::gcf()->width(m_nCvWidth);
	matplot::gcf()->height(m_nCvHeight);
	matplot::save(path + filename +"CV.png");
	std::cout << "Done" << std::endl;
}

void Grapher::GraphCIL(std::string sId, const T_CilData& data)
{
	// todo graph CIL
	if (data.vPulseWidths.size() <= 1)
	{
		std::cout << "Only one pulse width found - Skipping CIL Plot" << std::endl;
		return;
	}
	std::cout << "Rendering CIL plot..." << std::flush;

	std::cout << "Todo lol" << std::endl;

}

std::string Grapher::GetGraphPath(std::string sId, E_GraphType eType)
{
	std::string sFilename;
	switch (eType)
	{
	case E_GraphType::Eis: sFilename = "EIS/"; break;
	case E_GraphType::Cv: sFilename = "CV/"; break;
	case E_GraphType::Cil: sFilename = "CIL/"; break;
	}
	return m_PlotDir.string() + "/" + sId + "/Plots/" + sFilename + ".png";
}
