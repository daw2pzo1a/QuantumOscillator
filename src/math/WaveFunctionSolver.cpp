#include "WaveFunctionSolver.h"
#include "FiniteDifference.h"

#include <cmath>

WaveFunctionData WaveFunctionSolver::Calculate(
    const Grid& grid,
    double epsilon,
    double barrierStrength,
    int barrierIndex,
    double barrierSigma,
    int stateIndex
) const
{
    WaveFunctionData data;

    if (!grid.IsValid() || grid.GetPointCount() < 5)
        return data;
    
    double barrierPosition = 0.0;

    if (barrierIndex >= 0)
        barrierPosition = grid.GetPoint(barrierIndex);

    const int count = grid.GetPointCount();
    const double h = grid.GetStep();
    const int matchIndex = count / 2;

    std::vector<double> left(count, 0.0);
    std::vector<double> right(count, 0.0);

    left[0] = 0.0;
    left[1] = 1.0;

    for (int i = 1; i <= matchIndex; ++i)
    {
        const double xi = grid.GetPoint(i);
        const double A = FiniteDifference::CalculateA(
            xi,
            h,
            epsilon,
            barrierStrength,
            barrierPosition,
            barrierSigma
        );

        left[i + 1] = A * left[i] - left[i - 1];
    }

    right[count - 1] = 0.0;
    right[count - 2] = 1.0;

    for (int i = count - 2; i >= matchIndex + 1; --i)
    {
        const double xi = grid.GetPoint(i);
        const double A = FiniteDifference::CalculateA(
            xi,
            h,
            epsilon,
            barrierStrength,
            barrierPosition,
            barrierSigma
        );

        right[i - 1] = A * right[i] - right[i + 1];
    }

    const double denominator =
        right[matchIndex] * right[matchIndex] +
        right[matchIndex + 1] * right[matchIndex + 1];

    if (denominator < 1.0e-30)
        return data;

    const double scale =
        (
            left[matchIndex] * right[matchIndex] +
            left[matchIndex + 1] * right[matchIndex + 1]
        ) /
        denominator;

    data.stateIndex = stateIndex;
    data.energy = epsilon;
    data.x = grid.GetPoints();
    data.psi.resize(count);

    for (int i = 0; i <= matchIndex; ++i)
        data.psi[i] = left[i];

    for (int i = matchIndex + 1; i < count; ++i)
        data.psi[i] = scale * right[i];

    Normalize(data, h);

    return data;
}

void WaveFunctionSolver::Normalize(
    WaveFunctionData& data,
    double step
) const
{
    if (data.Empty())
        return;

    double integral = 0.0;

    for (int i = 0; i < data.Size() - 1; ++i)
    {
        const double leftValue =
            data.psi[i] * data.psi[i];

        const double rightValue =
            data.psi[i + 1] * data.psi[i + 1];

        integral +=
            0.5 *
            (leftValue + rightValue) *
            step;
    }

    if (integral <= 0.0)
        return;

    const double norm = std::sqrt(integral);

    for (double& value : data.psi)
        value /= norm;
}