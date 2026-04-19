#pragma once
#include <vector>

// sample StdDv
//  (sumsSq[key] - (sum * sum / n)) / (n - 1);
// population StdDv
//  (sumsSq[key] / n) - (mean * mean);

struct T_Stats
{
    double sum;
    double mean;
    double stddev;
};

template<typename T>
T_Stats stddev(const std::vector<T>& arr)
{
    T_Stats stats;
    stats.sum = 0;
    stats.stddev = 0;

    int size = static_cast<int>(arr.size());

    // Calculate the sum of elements in the vector
    for (int i = 0; i < size; ++i) {
        stats.sum += arr[i];
    }

    // Calculate the mean
    stats.mean = stats.sum / size;

    // Calculate the sum of squared differences from the
    // mean
    for (int i = 0; i < size; ++i) {
        stats.stddev += pow(arr[i] - stats.mean, 2);
    }

    // Calculate the square root of the variance to get the
    // standard deviation
    stats.stddev = sqrt(stats.stddev/size);
    return stats;
}


template<typename key_t, typename T>
std::map<key_t, T_Stats> stddev(const std::vector<std::map<key_t, T>>& maps)
{
    std::map<key_t, T_Stats> output;
    std::map<key_t, double> sumsSq;
    std::map<key_t, int> keyOccurances;

    for (const auto& map : maps)
    {
        for (const auto& [key, value] : map)
        {
            output[key].sum += value;
            sumsSq[key] += value * value;
            keyOccurances[key]++;
        }
    }

    for (auto& [key, stats] : output)
    {
        double mean = stats.sum / keyOccurances[key];
        stats.mean = mean;
        stats.stddev = std::sqrt((sumsSq[key] / keyOccurances[key]) - (mean * mean));
    }
    return output;
}