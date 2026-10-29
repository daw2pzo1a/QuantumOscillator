#include "FiniteDifference.h"
#include <cmath>

namespace FiniteDifference
{
    double CalculateA(
        double xi,
        double step,
        double epsilon,
        double barrierStrength,
        double barrierPosition,
        double barrierSigma
    )
    {
        double coefficient = 2.0 + step * step * (xi * xi - 2.0 * epsilon);

        if (barrierSigma > 0.0 && std::abs(barrierStrength) > 0.0)
        {
            const double pi = 3.14159265358979323846;
            const double difference = xi - barrierPosition;

            const double gaussian =
                std::exp(
                    -(difference * difference) /
                    (2.0 * barrierSigma * barrierSigma)
                ) /
                (barrierSigma * std::sqrt(2.0 * pi));

            coefficient +=
                2.0 *
                barrierStrength *
                step *
                step *
                gaussian;
        }

        return coefficient;
    }
}