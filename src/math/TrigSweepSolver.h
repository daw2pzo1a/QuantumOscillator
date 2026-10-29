#pragma once

#include "Grid.h"

class TrigSweepSolver
{
public:
    double CalculateRightPhase(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex, double barrierSigma) const;
    double CalculateRightPhaseOverPi(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex, double barrierSigma) const;

private:
    double UnwrapPhase(double phase, double previousPhase) const;

    static constexpr double kPI = 3.14159265358979323846;
};