#pragma once
#include <vector>

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

    int size = arr.size();

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