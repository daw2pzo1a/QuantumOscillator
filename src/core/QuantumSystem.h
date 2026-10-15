#pragma once

#include "SimulationConfig.h"

class QuantumSystem
{
public:
    explicit QuantumSystem(const SimulationConfig& config);

    void SetConfig(const SimulationConfig& config);

    bool IsValid() const;

    // Основные физические характеристики
    double GetOmega() const;
    double GetOscillatorLength() const;
    double GetEnergyScale() const;

    // Параметры дельта-барьера в безразмерной форме
    double GetDimensionlessBarrierPosition() const;
    double GetDimensionlessBarrierStrength() const;

    // Преобразования координаты
    double ToDimensionlessX(double x) const;
    double ToPhysicalX(double xi) const;

    // Преобразования энергии
    double ToDimensionlessEnergy(double energy) const;
    double ToPhysicalEnergy(double epsilon) const;

    // Потенциал гармонического осциллятора
    double GetPhysicalPotential(double x) const;
    double GetDimensionlessPotential(double xi) const;

private:
    SimulationConfig config_;

    static constexpr double HBAR = 1.054571817e-34;
};