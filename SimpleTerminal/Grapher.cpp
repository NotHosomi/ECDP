#define NOMINMAX
#include "Grapher.h"
#include <algorithm>
#include <matplot/matplot.h>

Grapher::Grapher(std::filesystem::path outputDir) :
	m_PlotDir(outputDir)
{

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

void Grapher::GraphCV(std::string path, std::string sId, T_CvData tData)
{
	std::map<double, double> loopAvrg;
	for (const auto& loop : tData.vLoops)
	{
		for (int i = 0; i < loop.vVoltages.size(); ++i)
		{
			if (!loopAvrg.contains(loop.vVoltages[i]))
			{

			}
			loopAvrg[] =
		}
	}
	std::cout << "Rendering CV plot... " << std::flush;
	using namespace matplot;

	figure(true);
	hold(false);



	std::string path = m_PlotDir.string() + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	save(path + "CV.png");
	std::cout << "Done\n" << std::endl;
}

void Grapher::GraphCIL(std::string path, std::string Id)
{
}
