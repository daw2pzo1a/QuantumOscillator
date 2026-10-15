#include "QuantumSystem.h"

#include <cmath>
#include <limits>

QuantumSystem::QuantumSystem(const SimulationConfig& config) : config_(config)
{
}

void QuantumSystem::SetConfig(const SimulationConfig& config)
{
    config_ = config;
}

bool QuantumSystem::IsValid() const
{
    return config_.mass > 0.0 && config_.potentialK > 0.0;
}

double QuantumSystem::GetOmega() const
{
    if (!IsValid())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return std::sqrt( 2.0 * config_.potentialK / config_.mass);
}

double QuantumSystem::GetOscillatorLength() const
{
    if (!IsValid())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double omega = GetOmega();

    return std::sqrt(HBAR / (config_.mass * omega));
}

double QuantumSystem::GetEnergyScale() const
{
    if (!IsValid())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return HBAR * GetOmega();
}

double QuantumSystem::GetDimensionlessBarrierPosition() const
{
    if (!IsValid())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double length = GetOscillatorLength();

    return config_.barrierPosition / length;
}

double QuantumSystem::GetDimensionlessBarrierStrength() const
{
    if (!IsValid())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double omega = GetOmega();

    const double length = GetOscillatorLength();

    return config_.barrierStrength / (HBAR * omega * length);
}

double QuantumSystem::ToDimensionlessX(double x) const
{
    return x / GetOscillatorLength();
}

double QuantumSystem::ToPhysicalX(double xi) const
{
    return xi * GetOscillatorLength();
}

double QuantumSystem::ToDimensionlessEnergy(double energy) const
{
    return energy / GetEnergyScale();
}

double QuantumSystem::ToPhysicalEnergy(double epsilon
) const
{
    return epsilon * GetEnergyScale();
}

double QuantumSystem::GetPhysicalPotential(double x) const
{
    return config_.potentialK * x * x;
}

double QuantumSystem::GetDimensionlessPotential(
    double xi
) const
{
    return 0.5 * xi * xi;
}