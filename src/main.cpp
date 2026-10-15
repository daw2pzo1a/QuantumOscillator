#include <raylib.h>

#include <imgui.h>
#include <implot.h>
#include <rlImGui.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/SimulationConfig.h"
#include "core/QuantumSystem.h"

int main()
{
    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(
        FLAG_WINDOW_RESIZABLE |
        FLAG_VSYNC_HINT |
        FLAG_MSAA_4X_HINT
    );

    InitWindow(
        screenWidth,
        screenHeight,
        "Quantum Oscillator"
    );

    SetTargetFPS(144);

    rlImGuiSetup(true);

    ImPlot::CreateContext();

    // --------------------------------------------------------
    // Параметры нашей физической системы.
    // --------------------------------------------------------

    SimulationConfig config;

    QuantumSystem system(config);

    // Для отображения потенциала.
    const int plotPointCount = 800;

    std::vector<double> xi(plotPointCount);
    std::vector<double> potential(plotPointCount);

    while (!WindowShouldClose())
    {
        // ----------------------------------------------------
        // Передаем текущие параметры системе.
        // ----------------------------------------------------

        system.SetConfig(config);

        // ----------------------------------------------------
        // Строим данные потенциала V(xi) = xi^2 / 2.
        // ----------------------------------------------------

        if (config.xiRight > config.xiLeft)
        {
            for (int i = 0; i < plotPointCount; ++i)
            {
                const double t =
                    static_cast<double>(i) /
                    static_cast<double>(
                        plotPointCount - 1
                    );

                xi[i] =
                    config.xiLeft +
                    t *
                    (
                        config.xiRight -
                        config.xiLeft
                    );

                potential[i] =
                    system.GetDimensionlessPotential(
                        xi[i]
                    );
            }
        }

        BeginDrawing();

        ClearBackground(
            Color{25, 25, 30, 255}
        );

        rlImGuiBegin();

        ImGui::SetNextWindowPos(
            ImVec2(0.0f, 0.0f),
            ImGuiCond_Always
        );

        ImGui::SetNextWindowSize(
            ImVec2(
                static_cast<float>(
                    GetScreenWidth()
                ),
                static_cast<float>(
                    GetScreenHeight()
                )
            ),
            ImGuiCond_Always
        );

        ImGui::Begin(
            "Quantum Oscillator",
            nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse
        );

        // ====================================================
        // ФИЗИЧЕСКИЕ ПАРАМЕТРЫ
        // ====================================================

        ImGui::Text(
            "Physical parameters"
        );

        ImGui::Separator();

        ImGui::InputDouble(
            "Mass m [kg]",
            &config.mass,
            0.0,
            0.0,
            "%.6e"
        );

        ImGui::InputDouble(
            "Potential k [J/m^2]",
            &config.potentialK,
            0.0,
            0.0,
            "%.6e"
        );

        ImGui::InputDouble(
            "Barrier Lambda [J*m]",
            &config.barrierStrength,
            0.0,
            0.0,
            "%.6e"
        );

        ImGui::InputDouble(
            "Barrier position a [m]",
            &config.barrierPosition,
            0.0,
            0.0,
            "%.6e"
        );

        ImGui::Spacing();

        // ====================================================
        // ЧИСЛЕННАЯ ОБЛАСТЬ
        // ====================================================

        ImGui::Text(
            "Dimensionless calculation area"
        );

        ImGui::Separator();

        ImGui::InputDouble(
            "xi left",
            &config.xiLeft
        );

        ImGui::InputDouble(
            "xi right",
            &config.xiRight
        );

        ImGui::Spacing();

        // ====================================================
        // ПРОВЕРКА ПАРАМЕТРОВ
        // ====================================================

        if (!system.IsValid())
        {
            ImGui::TextColored(
                ImVec4(
                    1.0f,
                    0.3f,
                    0.3f,
                    1.0f
                ),
                "Error: mass and k must be greater than zero."
            );
        }
        else
        {
            // =================================================
            // ПРОИЗВОДНЫЕ ВЕЛИЧИНЫ
            // =================================================

            const double omega =
                system.GetOmega();

            const double length =
                system.GetOscillatorLength();

            const double energyScale =
                system.GetEnergyScale();

            const double alpha =
                system.GetDimensionlessBarrierPosition();

            const double g =
                system.GetDimensionlessBarrierStrength();

            ImGui::Text(
                "Calculated values"
            );

            ImGui::Separator();

            ImGui::Text(
                "omega = %.6e rad/s",
                omega
            );

            ImGui::Text(
                "oscillator length l = %.6e m",
                length
            );

            ImGui::Text(
                "energy scale hbar*omega = %.6e J",
                energyScale
            );

            ImGui::Text(
                "dimensionless barrier position alpha = %.6f",
                alpha
            );

            ImGui::Text(
                "dimensionless barrier strength g = %.6f",
                g
            );

            ImGui::Spacing();

            // =================================================
            // ГРАФИК ПОТЕНЦИАЛА
            // =================================================

            if (
                config.xiRight >
                config.xiLeft
            )
            {
                if (ImPlot::BeginPlot(
                    "Dimensionless potential",
                    ImVec2(-1, -1)
                ))
                {
                    ImPlot::SetupAxes(
                        "xi",
                        "V(xi)"
                    );

                    ImPlot::PlotLine(
                        "V(xi) = xi^2 / 2",
                        xi.data(),
                        potential.data(),
                        plotPointCount
                    );

                    // -----------------------------------------
                    // Рисуем дельта-барьер вертикальной линией.
                    // -----------------------------------------

                    if (
                        alpha >= config.xiLeft &&
                        alpha <= config.xiRight
                    )
                    {
                        const double maxAbsXi =
                            std::max(
                                std::abs(
                                    config.xiLeft
                                ),
                                std::abs(
                                    config.xiRight
                                )
                            );

                        const double maxPotential =
                            0.5 *
                            maxAbsXi *
                            maxAbsXi;

                        double barrierX[2] =
                        {
                            alpha,
                            alpha
                        };

                        double barrierY[2] =
                        {
                            0.0,
                            maxPotential
                        };

                        ImPlot::PlotLine(
                            "delta barrier",
                            barrierX,
                            barrierY,
                            2
                        );
                    }

                    ImPlot::EndPlot();
                }
            }
            else
            {
                ImGui::TextColored(
                    ImVec4(
                        1.0f,
                        0.3f,
                        0.3f,
                        1.0f
                    ),
                    "Error: xi right must be greater than xi left."
                );
            }
        }

        ImGui::End();

        rlImGuiEnd();

        EndDrawing();
    }

    ImPlot::DestroyContext();

    rlImGuiShutdown();

    CloseWindow();

    return 0;
}