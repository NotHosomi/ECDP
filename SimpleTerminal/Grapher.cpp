#include "Grapher.h"
#include <matplot/matplot.h>

Grapher::Grapher(std::filesystem::path outputDir) :
	m_PlotDir(outputDir)
{

}

void Grapher::GraphEIS(std::string sId, T_ErrorBarF tZ, T_ErrorBarF tPhase)
{
	matplot::figure_handle fig = matplot::figure();
	matplot::axes_handle magnitudePlot = fig->add_axes();
	matplot::axes_handle phasePlot = fig->add_axes();

	magnitudePlot->errorbar(tZ.x, tZ.y, tZ.err);
	phasePlot->errorbar(tPhase.x, tPhase.y, tPhase.err);

	// not sure abt this block. Do they both need it? I think cuz they have the same X vals it should be fine
	magnitudePlot->x_axis().scale(matplot::axis_type::axis_scale::log);
	phasePlot->x_axis().scale(matplot::axis_type::axis_scale::log);

	magnitudePlot->y_axis().scale(matplot::axis_type::axis_scale::log);
	phasePlot->ylim({ -90, 0 });

	matplot::save(m_PlotDir.string() + "/" + sId + "/EIS.png");
}

void Grapher::GraphCV(std::string path, std::string Id)
{
}

void Grapher::GraphCIL(std::string path, std::string Id)
{
}
