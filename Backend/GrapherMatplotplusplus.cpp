#define NOMINMAX
#include "GrapherMatplotplusplus.h"
#include <algorithm>
#include <matplot/matplot.h>
#include "Term.h"
#include "CvData.h"
#include "CilData.h"
#include "Ingester.h"
#include "CsvFile.h"
#include "Options.h"

void GrapherMatplotplusplus::EisAverage(const std::string& sId, const T_ErrorPlotF& tZ, const T_ErrorPlotF& tPhase, bool bReplot)
{
	std::string path = m_sOutputPath + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	std::string file = path + "EIS.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	Term::Get()->Print("Rendering EIS plot...");
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

	save(file);
	Term::Get()->Println("Done\n");
}

void GrapherMatplotplusplus::EisSingle(const std::string& sId, const std::string& filename, const T_EisRawData& tRaw, bool bReplot)
{
	Term::Get()->Println("EisSingle not implemented yet with Internal backend");
}

void GrapherMatplotplusplus::CvAverage(const std::string& sId, T_ErrorPlotF tLoop, bool bReplot)
{
	std::string path = m_sOutputPath + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	std::string file = path + "CV.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}

	if (tLoop.x.size() == 0)
	{
		Term::Get()->Println("Cannot render CV plot");
		return;
	}
	using namespace matplot;
	Term::Get()->Print("Rendering CV plot...");

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
	gca()->x_axis().label_font_size(28);
	gca()->y_axis().label_font_size(28);
	gca()->font_size(30);

	matplot::xlim({ -0.65, 0.85 });
	matplot::ylim({ -3e-6, 3e-6 });

	matplot::gcf()->width(m_nCvWidth);
	matplot::gcf()->height(m_nCvHeight);
	save(file);
	Term::Get()->Println("Done\n");
}

void GrapherMatplotplusplus::CvSingle(const std::string& sId, const std::string& filename, T_CvElectrodeData tElectrode, bool bReplot)
{
	std::string path = std::filesystem::current_path().string() + m_sOutputPath + "/" + sId + "/Plots/CV/";;
	std::filesystem::create_directories(path);
	std::string file = path + filename + " CV.png";
	if (std::filesystem::exists(file) || Options::Get().GetOpt<bool>("plotter-force-replot") || bReplot)
	{
		return;
	}
	Term::Get()->Print("Rendering CV plot (" + filename + ")... ");

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
	matplot::ylim({ -3e-6, 3e-6 });

	matplot::gcf()->width(m_nCvWidth);
	matplot::gcf()->height(m_nCvHeight);
	matplot::save(file);
	Term::Get()->Println("Done\n");
}

void GrapherMatplotplusplus::CilAverage(const std::string& sId, const T_ErrorPlotF& data, bool bReplot)
{
	Term::Get()->Println("CilAverage not implemented yet with Internal plotting backend");
}

void GrapherMatplotplusplus::CilMulti(const std::string& sId, const T_CilData& data, bool bReplot)
{
	Term::Get()->Println("CilMulti not implemented yet with Internal plotting backend");
}

