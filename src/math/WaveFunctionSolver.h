#pragma once

#include "Grid.h"
#include "../data/WaveFunctionData.h"

class WaveFunctionSolver
{
public:
    WaveFunctionData Calculate(
        const Grid& grid,
        double epsilon,
        double barrierStrength,
        int barrierIndex,
        double barrierSigma,
        int stateIndex
    ) const;

private:
    void Normalize(WaveFunctionData& data, double step) const;
};