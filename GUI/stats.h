#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

namespace Stats
{
	template <typename T>
	T Mean(std::vector<T> data)
	{
		T sum = std::accumulate(data.begin(), data.end(), static_cast<T>(0));
		return sum / data.size();
	};

	template <typename T>
	T Stddev(std::vector<T> data)
	{
		T mean = Stats::Mean(data);
		std::vector<T> delta(data.size());
		std::transform(data.begin(), data.end(), delta.begin(), [mean](T x) { return x - mean; });
		T sq_sum = std::inner_product(delta.begin(), delta.end(), delta.begin(), static_cast<T>(0));
		return std::sqrt(sq_sum / data.size());
	};
}