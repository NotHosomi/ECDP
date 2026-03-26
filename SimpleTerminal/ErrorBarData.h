#pragma once
#include <vector>

template <typename X_t, typename Y_t, typename Err_t>
struct T_ErrorBar
{
	std::vector<X_t> x;
	std::vector<Y_t> y;
	std::vector<Err_t> err;
};

using T_ErrorBarF = T_ErrorBar<float, float, float>;
