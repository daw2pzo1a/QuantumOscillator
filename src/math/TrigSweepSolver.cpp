#include "TrigSweepSolver.h"
#include "FiniteDifference.h"

#include <cmath>
#include <limits>

double TrigSweepSolver::CalculateRightPhase(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex) const
{
    if (!grid.IsValid() || grid.GetPointCount() < 3)
        return std::numeric_limits<double>::quiet_NaN();

    double theta = kPI / 2.0;
    const double h = grid.GetStep();

    for (int i = 1; i < grid.GetPointCount() - 1; ++i)
    {
        const double xi = grid.GetPoint(i);
        const bool barrierNode = i == barrierIndex;
        const double A = FiniteDifference::CalculateA(xi, h, epsilon, barrierStrength, barrierNode);

        const double sinTheta = std::sin(theta);
        const double cosTheta = std::cos(theta);
        const double nextPsi = A * sinTheta - cosTheta;
        const double rawPhase = std::atan2(nextPsi, sinTheta);

        theta = UnwrapPhase(rawPhase, theta);
    }

    return theta;
}

double TrigSweepSolver::CalculateRightPhaseOverPi(const Grid& grid, double epsilon, double barrierStrength, int barrierIndex) const
{
    return CalculateRightPhase(grid, epsilon, barrierStrength, barrierIndex) / kPI;
}

double TrigSweepSolver::UnwrapPhase(double phase, double previousPhase) const
{
    const double shift = std::round((previousPhase - phase) / kPI);
    return phase + shift * kPI;
}