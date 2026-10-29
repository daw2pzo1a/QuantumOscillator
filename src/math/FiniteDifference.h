#pragma once

namespace FiniteDifference
{
    double CalculateA(
        double xi,
        double step,
        double epsilon,
        double barrierStrength,
        double barrierPosition,
        double barrierSigma
    );
}