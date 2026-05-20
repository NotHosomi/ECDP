#pragma once
#include <vector>

template <typename X_t, typename Y_t, typename Err_t>
struct T_ErrorPlot
{
	std::vector<X_t> x;
	std::vector<Y_t> y;
	std::vector<Err_t> err;
};

using T_ErrorPlotF = T_ErrorPlot<float, float, float>;
using T_ErrorPlotD = T_ErrorPlot<double, double, double>;


template <typename X_t, typename Y_t, typename Err_t>
struct T_LinePlot
{
	std::vector<X_t> x;
	std::vector<Y_t> y;
};

using T_LinePlotF = T_LinePlot<float, float, float>;
using T_LinePlotD = T_LinePlot<double, double, double>;