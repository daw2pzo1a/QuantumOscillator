#include <raylib.h>
#include <imgui.h>
#include <implot.h>
#include <rlImGui.h>
#include <algorithm>
#include <cmath>
#include <vector>

#include "core/SimulationConfig.h"
#include "core/QuantumSystem.h"
#include "math/Grid.h"
#include "math/FiniteDifference.h"
#include "math/TrigSweepSolver.h"

int main()
{
    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Quantum Oscillator");
    SetTargetFPS(144);

    rlImGuiSetup(true);
    ImPlot::CreateContext();

    SimulationConfig config;
    QuantumSystem system(config);
    Grid grid;
    TrigSweepSolver trigSweepSolver;
    double testEnergy = 0.5;

    const int plotPointCount = 800;
    std::vector<double> xi(plotPointCount);
    std::vector<double> potential(plotPointCount);

    while (!WindowShouldClose())
    {
        system.SetConfig(config);
        grid.Build(config.xiLeft, config.xiRight, config.gridPoints);

        if (config.xiRight > config.xiLeft)
        {
            for (int i = 0; i < plotPointCount; ++i)
            {
                const double t = static_cast<double>(i) / static_cast<double>(plotPointCount - 1);
                xi[i] = config.xiLeft + t * (config.xiRight - config.xiLeft);
                potential[i] = system.GetDimensionlessPotential(xi[i]);
            }
        }

        BeginDrawing();
        ClearBackground(Color{25, 25, 30, 255});
        rlImGuiBegin();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())), ImGuiCond_Always);

        ImGui::Begin("Quantum Oscillator", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::BeginChild("Control Panel", ImVec2(460.0f, 0.0f), true);

        ImGui::Text("Physical parameters");
        ImGui::Separator();

        ImGui::InputDouble("Mass m [kg]", &config.mass, 0.0, 0.0, "%.6e");
        ImGui::InputDouble("Potential k [J/m^2]", &config.potentialK, 0.0, 0.0, "%.6e");
        ImGui::InputDouble("Barrier Lambda [J*m]", &config.barrierStrength, 0.0, 0.0, "%.6e");
        ImGui::InputDouble("Barrier position a [m]", &config.barrierPosition, 0.0, 0.0, "%.6e");

        ImGui::Spacing();
        ImGui::Text("Dimensionless calculation area");
        ImGui::Separator();

        ImGui::InputDouble("xi left", &config.xiLeft);
        ImGui::InputDouble("xi right", &config.xiRight);
        ImGui::InputInt("Grid points", &config.gridPoints);

        ImGui::Spacing();

        if (!system.IsValid())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: mass and k must be greater than zero.");
        }
        else
        {
            const double omega = system.GetOmega();
            const double length = system.GetOscillatorLength();
            const double energyScale = system.GetEnergyScale();
            const double alpha = system.GetDimensionlessBarrierPosition();
            const double g = system.GetDimensionlessBarrierStrength();
            const int barrierIndex = grid.FindNearestIndex(alpha);

            ImGui::Text("Calculated values");
            ImGui::Separator();

            ImGui::Text("omega = %.6e rad/s", omega);
            ImGui::Text("oscillator length l = %.6e m", length);
            ImGui::Text("energy scale hbar*omega = %.6e J", energyScale);
            ImGui::Text("dimensionless barrier position alpha = %.6f", alpha);
            ImGui::Text("dimensionless barrier strength g = %.6f", g);

            ImGui::Spacing();
            ImGui::Text("Grid");
            ImGui::Separator();

            if (grid.IsValid())
            {
                ImGui::Text("points N = %d", grid.GetPointCount());
                ImGui::Text("step h = %.10f", grid.GetStep());

                if (barrierIndex >= 0)
                {
                    const double actualBarrierPosition = grid.GetPoint(barrierIndex);
                    const double barrierError = actualBarrierPosition - alpha;

                    ImGui::Text("barrier index j = %d", barrierIndex);
                    ImGui::Text("requested alpha = %.10f", alpha);
                    ImGui::Text("grid alpha = %.10f", actualBarrierPosition);
                    ImGui::Text("position error = %.6e", barrierError);
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Barrier is outside the calculation area.");
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid grid parameters.");
            }

            ImGui::Spacing();
            ImGui::Text("Finite difference test");
            ImGui::Separator();

            ImGui::InputDouble("Test epsilon", &testEnergy);

            if (grid.IsValid())
            {
                const int centerIndex = grid.FindNearestIndex(0.0);

                if (centerIndex >= 0)
                {
                    const double centerXi = grid.GetPoint(centerIndex);
                    const bool centerIsBarrier = centerIndex == barrierIndex;
                    const double centerA = FiniteDifference::CalculateA(centerXi, grid.GetStep(), testEnergy, g, centerIsBarrier);

                    ImGui::Text("center index = %d", centerIndex);
                    ImGui::Text("center xi = %.10f", centerXi);
                    ImGui::Text("A(center) = %.12f", centerA);
                }

                if (barrierIndex >= 0)
                {
                    const double barrierXi = grid.GetPoint(barrierIndex);
                    const double barrierA = FiniteDifference::CalculateA(barrierXi, grid.GetStep(), testEnergy, g, true);

                    ImGui::Text("A(barrier) = %.12f", barrierA);
                }
            }

            ImGui::Spacing();
            ImGui::Text("Trigonometric sweep test");
            ImGui::Separator();

            if (grid.IsValid())
            {
                const double rightPhase = trigSweepSolver.CalculateRightPhase(grid, testEnergy, g, barrierIndex);
                const double rightPhaseOverPi = trigSweepSolver.CalculateRightPhaseOverPi(grid, testEnergy, g, barrierIndex);

                ImGui::Text("right phase theta = %.12f rad", rightPhase);
                ImGui::Text("theta / pi = %.12f", rightPhaseOverPi);
            }
        }

        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginChild("Graph", ImVec2(0.0f, 0.0f), true);
        ImGui::Text("Dimensionless potential");

        if (!system.IsValid())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Graph unavailable: fix physical parameters.");
        }
        else if (config.xiRight <= config.xiLeft)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: xi right must be greater than xi left.");
        }
        else
        {
            const double alpha = system.GetDimensionlessBarrierPosition();
            const double g = system.GetDimensionlessBarrierStrength();

            if (ImPlot::BeginPlot("Dimensionless potential", ImVec2(-1.0f, -1.0f)))
            {
                ImPlot::SetupAxes("xi", "V(xi)");
                ImPlot::PlotLine("V(xi) = xi^2 / 2", xi.data(), potential.data(), plotPointCount);

                if (alpha >= config.xiLeft && alpha <= config.xiRight && std::abs(g) > 1.0e-14)
                {
                    const double maxAbsXi = std::max(std::abs(config.xiLeft), std::abs(config.xiRight));
                    const double maxPotential = 0.5 * maxAbsXi * maxAbsXi;
                    double barrierX[2] = {alpha, alpha};
                    double barrierY[2] = {0.0, maxPotential};

                    ImPlot::PlotLine("delta barrier", barrierX, barrierY, 2);
                }

                ImPlot::EndPlot();
            }
        }

        ImGui::EndChild();
        ImGui::End();

        rlImGuiEnd();
        EndDrawing();
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}