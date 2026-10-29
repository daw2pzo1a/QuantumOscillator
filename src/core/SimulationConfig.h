#pragma once

struct SimulationConfig
{
    // Физические параметры

    // Масса частицы, кг.
    double mass = 9.1093837015e-31;

    // Коэффициент потенциала U(x) = k*x^2
    // Единица измерения: Дж/м^2
    double potentialK = 1.0e-2;

    // Сила дельта-барьера Lambda
    // Единица измерения: Дж*м
    double barrierStrength = 1.0e-29;

    // Положение дельта-барьера, м
    double barrierPosition = 0.0;

    // Ширина дельта-барьера, м
    double barrierSigma = 0.05;

    // Численные параметры
    // Границы расчета уже в безразмерной координате xi
    double xiLeft = -8.0;
    double xiRight = 8.0;

    // Количество точек пространственной сетки
    int gridPoints = 1601;

    // Диапазон поиска безразмерной энергии epsilon
    double energyMin = 0.0;
    double energyMax = 8.0;

    // Количество пробных значений энергии.
    int energySamples = 800;

    // Выбранное квантовое состояние.
    // 0 - основное, 1 - первое возбужденное и т.д
    int stateIndex = 0;

    // Точность поиска собственного значения
    double tolerance = 1.0e-10;
};