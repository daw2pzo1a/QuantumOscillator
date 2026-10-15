#include "SpectrumSolver.h"
#include <cmath>

SpectrumData SpectrumSolver::Calculate(const Grid& grid, double energyMin, double energyMax, int energySamples, double barrierStrength, int barrierIndex, double tolerance) const
{
    SpectrumData data;

    if (!grid.IsValid() || energyMax <= energyMin || energySamples < 2 || tolerance <= 0.0)
        return data;

    data.energies.reserve(energySamples);
    data.phasesOverPi.reserve(energySamples);

    const double energyStep = (energyMax - energyMin) / static_cast<double>(energySamples - 1);

    double previousEnergy = energyMin;
    double previousResidual = BoundaryResidual(grid, previousEnergy, barrierStrength, barrierIndex);

    for (int i = 0; i < energySamples; ++i)
    {
        const double epsilon = energyMin + static_cast<double>(i) * energyStep;
        const double phaseOverPi = trigSweepSolver_.CalculateRightPhaseOverPi(grid, epsilon, barrierStrength, barrierIndex);

        data.energies.push_back(epsilon);
        data.phasesOverPi.push_back(phaseOverPi);

        if (i == 0)
            continue;

        const double residual = BoundaryResidual(grid, epsilon, barrierStrength, barrierIndex);

        if (previousResidual * residual < 0.0)
        {
            const double eigenvalue = FindEigenvalue(grid, previousEnergy, epsilon, barrierStrength, barrierIndex, tolerance);

            if (data.eigenvalues.empty() || std::abs(eigenvalue - data.eigenvalues.back()) > tolerance * 10.0)
                data.eigenvalues.push_back(eigenvalue);
        }

        previousEnergy = epsilon;
        previousResidual = residual;
    }

    return data;
}

double SpectrumSolver::BoundaryResidual(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex) const
{
    const double phase = trigSweepSolver_.CalculateRightPhase(grid, epsilon, barrierStrength, barrierIndex);
    return std::sin(phase);
}

double SpectrumSolver::FindEigenvalue(const Grid& grid, double leftEnergy, double rightEnergy, double barrierStrength, int barrierIndex, double tolerance) const
{
    double left = leftEnergy;
    double right = rightEnergy;
    double leftResidual = BoundaryResidual(grid, left, barrierStrength, barrierIndex);

    for (int iteration = 0; iteration < 100; ++iteration)
    {
        const double middle = 0.5 * (left + right);
        const double middleResidual = BoundaryResidual(grid, middle, barrierStrength, barrierIndex);

        if (std::abs(right - left) < tolerance)
            return middle;

        if (leftResidual * middleResidual <= 0.0)
        {
            right = middle;
        }
        else
        {
            left = middle;
            leftResidual = middleResidual;
        }
    }

    return 0.5 * (left + right);
}