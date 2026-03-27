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
	using namespace matplot;
	error_bar_handle magnitudePlot = matplot::errorbar(tZ.x, tZ.y, tZ.err);
	gca()->x_axis().scale(matplot::axis_type::axis_scale::log);
	gca()->y_axis().scale(matplot::axis_type::axis_scale::log);

	double y1Min = std::min(1000.0, gca()->ylim()[0] * 0.9);
	double y1Max = std::max(100000.0, gca()->ylim()[1] * 1.1);
	 
	// generate the scaled error bar, because use_y2 breaks the errorbars if the new scale doesn't match y1
	double scalefactor = (y1Max - y1Min) / 90.0;
	std::vector<double> scaledY;
	std::vector<double> scaledErr;
	for (int i = 0; i < tPhase.y.size(); ++i)
	{
		scaledY.push_back((tPhase.y[i]) * scalefactor + y1Min);
		scaledErr.push_back(tPhase.err[i] * scalefactor);
	}

	hold(true);
	//error_bar_handle phasePlot = errorbar(tPhase.x, tPhase.y, tPhase.y);
	error_bar_handle phasePlot = errorbar(tPhase.x, scaledY, scaledErr);
	//phasePlot->use_y2(true);
	//y2lim({ 0,90 });
	
	std::string path = m_PlotDir.string() + "/" + sId + "/Plots/";
	std::filesystem::create_directories(path);
	save(path + "EIS.png");
}

void Grapher::GraphCV(std::string path, std::string Id)
{
}

void Grapher::GraphCIL(std::string path, std::string Id)
{
}
