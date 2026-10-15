#pragma once

#include "Grid.h"

class TrigSweepSolver
{
public:
    double CalculateRightPhase(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex) const;
    double CalculateRightPhaseOverPi(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex) const;

private:
    double UnwrapPhase(double phase, double previousPhase) const;

    static constexpr double kPI = 3.14159265358979323846;
};