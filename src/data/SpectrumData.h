#pragma once
#include <vector>

struct SpectrumData
{
    std::vector<double> energies;
    std::vector<double> phasesOverPi;
    std::vector<double> eigenvalues;

    void Clear()
    {
        energies.clear();
        phasesOverPi.clear();
        eigenvalues.clear();
    }

    bool Empty() const
    {
        return energies.empty() || phasesOverPi.empty();
    }

    int Size() const
    {
        return static_cast<int>(energies.size());
    }
};