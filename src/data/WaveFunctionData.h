#pragma once
#include <vector>

struct WaveFunctionData
{
    int stateIndex = -1;
    double energy = 0.0;
    std::vector<double> x;
    std::vector<double> psi;

    void Clear()
    {
        stateIndex = -1;
        energy = 0.0;
        x.clear();
        psi.clear();
    }

    bool Empty() const
    {
        return x.empty() || psi.empty();
    }

    int Size() const
    {
        return static_cast<int>(x.size());
    }
};