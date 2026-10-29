#pragma once
#include "Grid.h"
#include "TrigSweepSolver.h"
#include "../data/SpectrumData.h"

class SpectrumSolver
{
public:
    SpectrumData Calculate(const Grid& grid, double energyMin, double energyMax, int energySamples, 
        double barrierStrength, int barrierIndex, double barrierSigma, double tolerance) const;

private:
    double BoundaryResidual(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex, double barrierSigma) const;
    double FindEigenvalue(const Grid& grid, double leftEnergy, double rightEnergy, double barrierStrength, int barrierIndex, 
        double barrierSigma, double tolerance) const;

    TrigSweepSolver trigSweepSolver_;
};