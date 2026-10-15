#include "FiniteDifference.h"

namespace FiniteDifference
{
    double CalculateA(double xi, double step, double epsilon, double barrierStrength, bool barrierNode)
    {
        double coefficient = 2.0 + step * step * (xi * xi - 2.0 * epsilon);

        if (barrierNode)
            coefficient += 2.0 * barrierStrength * step;

        return coefficient;
    }
}