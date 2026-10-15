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
#include "math/SpectrumSolver.h"
#include "math/WaveFunctionSolver.h"
#include "data/SpectrumData.h"
#include "data/WaveFunctionData.h"

int main()
{
    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Квантовый осциллятор");
    SetWindowMinSize(1200, 700);
    SetExitKey(KEY_NULL);
    SetTargetFPS(144);

    rlImGuiBeginInitImGui();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

    if (FileExists(fontPath))
    {
        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f);
        if (font != nullptr)
            io.FontDefault = font;
    }

    rlImGuiEndInitImGui();
    ImPlot::CreateContext();

    SimulationConfig config;
    QuantumSystem system(config);
    Grid grid;
    TrigSweepSolver trigSweepSolver;
    SpectrumSolver spectrumSolver;
    WaveFunctionSolver waveFunctionSolver;
    SpectrumData spectrumData;
    WaveFunctionData waveFunctionData;

    double testEnergy = 0.5;

    const int plotPointCount = 800;
    std::vector<double> xi(plotPointCount);
    std::vector<double> potential(plotPointCount);

    bool running = true;
    bool exitRequested = false;
    bool systemCloseHandled = false;

    while (running)
    {
        if (WindowShouldClose() && !systemCloseHandled)
        {
            exitRequested = true;
            systemCloseHandled = true;
        }

        if (IsKeyPressed(KEY_ESCAPE))
            exitRequested = true;

        BeginDrawing();
        ClearBackground(Color{25, 25, 30, 255});
        rlImGuiBegin();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())), ImGuiCond_Always);

        ImGui::Begin(
            "Квантовый осциллятор",
            nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse
        );

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float leftPanelWidth = std::clamp(availableWidth * 0.36f, 380.0f, 520.0f);

        ImGui::BeginChild("Панель управления", ImVec2(leftPanelWidth, 0.0f), true);

        bool parametersChanged = false;

        if (ImGui::CollapsingHeader("Физические параметры", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Масса m, кг");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##mass", &config.mass, 0.0, 0.0, "%.6e");

            ImGui::Text("Коэффициент k, Дж/м^2");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##potentialK", &config.potentialK, 0.0, 0.0, "%.6e");

            ImGui::Text("Сила барьера Lambda, Дж*м");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##barrierStrength", &config.barrierStrength, 0.0, 0.0, "%.6e");

            ImGui::Text("Положение барьера a, м");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##barrierPosition", &config.barrierPosition, 0.0, 0.0, "%.6e");
        }

        if (ImGui::CollapsingHeader("Расчетная область", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Левая граница xi");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##xiLeft", &config.xiLeft);

            ImGui::Text("Правая граница xi");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##xiRight", &config.xiRight);

            ImGui::Text("Количество точек сетки");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputInt("##gridPoints", &config.gridPoints);
        }

        if (ImGui::CollapsingHeader("Поиск спектра", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Минимальная энергия");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##energyMin", &config.energyMin);

            ImGui::Text("Максимальная энергия");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##energyMax", &config.energyMax);

            ImGui::Text("Количество пробных энергий");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputInt("##energySamples", &config.energySamples);

            ImGui::Text("Точность");
            ImGui::SetNextItemWidth(-1.0f);
            parametersChanged |= ImGui::InputDouble("##tolerance", &config.tolerance, 0.0, 0.0, "%.3e");
        }

        if (parametersChanged)
        {
            spectrumData.Clear();
            waveFunctionData.Clear();
        }

        system.SetConfig(config);
        grid.Build(config.xiLeft, config.xiRight, config.gridPoints);

        const bool systemValid = system.IsValid();
        const bool rangeValid = config.xiRight > config.xiLeft;
        const bool gridValid = grid.IsValid() && config.gridPoints >= 5;
        const bool spectrumInputValid =
            systemValid &&
            rangeValid &&
            gridValid &&
            config.energyMax > config.energyMin &&
            config.energySamples >= 2 &&
            config.tolerance > 0.0;

        double alpha = 0.0;
        double g = 0.0;
        int barrierIndex = -1;

        if (systemValid)
        {
            alpha = system.GetDimensionlessBarrierPosition();
            g = system.GetDimensionlessBarrierStrength();

            if (gridValid)
                barrierIndex = grid.FindNearestIndex(alpha);
        }

        if (!systemValid)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Ошибка: масса и коэффициент k должны быть больше нуля.");
        }
        else
        {
            if (ImGui::CollapsingHeader("Расчетные величины", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("omega = %.6e рад/с", system.GetOmega());
                ImGui::TextWrapped("Осцилляторная длина l = %.6e м", system.GetOscillatorLength());
                ImGui::TextWrapped("Масштаб энергии hbar*omega = %.6e Дж", system.GetEnergyScale());
                ImGui::TextWrapped("Безразмерное положение alpha = %.6f", alpha);
                ImGui::TextWrapped("Безразмерная сила g = %.6f", g);
            }

            if (ImGui::CollapsingHeader("Сетка", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (gridValid)
                {
                    ImGui::Text("Количество точек N = %d", grid.GetPointCount());
                    ImGui::Text("Шаг h = %.10f", grid.GetStep());

                    if (barrierIndex >= 0)
                    {
                        const double actualBarrierPosition = grid.GetPoint(barrierIndex);
                        const double barrierError = actualBarrierPosition - alpha;

                        ImGui::Text("Индекс барьера j = %d", barrierIndex);
                        ImGui::TextWrapped("Заданное alpha = %.10f", alpha);
                        ImGui::TextWrapped("Положение на сетке = %.10f", actualBarrierPosition);
                        ImGui::TextWrapped("Ошибка положения = %.6e", barrierError);
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Барьер находится вне расчетной области.");
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Ошибка: нужно не менее 5 точек и правильные границы.");
                }
            }

            if (ImGui::CollapsingHeader("Проверка метода", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Пробная энергия epsilon");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputDouble("##testEnergy", &testEnergy);

                if (gridValid)
                {
                    const int centerIndex = grid.FindNearestIndex(0.0);

                    if (centerIndex >= 0)
                    {
                        const double centerXi = grid.GetPoint(centerIndex);
                        const bool centerIsBarrier = centerIndex == barrierIndex;
                        const double centerA = FiniteDifference::CalculateA(centerXi, grid.GetStep(), testEnergy, g, centerIsBarrier);

                        ImGui::Text("Индекс центра = %d", centerIndex);
                        ImGui::Text("xi в центре = %.10f", centerXi);
                        ImGui::Text("A в центре = %.12f", centerA);
                    }

                    if (barrierIndex >= 0)
                    {
                        const double barrierXi = grid.GetPoint(barrierIndex);
                        const double barrierA = FiniteDifference::CalculateA(barrierXi, grid.GetStep(), testEnergy, g, true);
                        ImGui::Text("A в точке барьера = %.12f", barrierA);
                    }

                    const double rightPhase = trigSweepSolver.CalculateRightPhase(grid, testEnergy, g, barrierIndex);
                    const double rightPhaseOverPi = trigSweepSolver.CalculateRightPhaseOverPi(grid, testEnergy, g, barrierIndex);

                    ImGui::TextWrapped("Фаза справа theta = %.12f рад", rightPhase);
                    ImGui::TextWrapped("theta / pi = %.12f", rightPhaseOverPi);
                }
            }

            if (ImGui::CollapsingHeader("Спектр", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (!spectrumInputValid)
                {
                    if (config.energyMax <= config.energyMin)
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Energy max должна быть больше Energy min.");

                    if (config.energySamples < 2)
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Количество пробных энергий должно быть не меньше 2.");

                    if (config.tolerance <= 0.0)
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Точность должна быть больше нуля.");
                }

                ImGui::BeginDisabled(!spectrumInputValid);

                if (ImGui::Button("Рассчитать спектр", ImVec2(-1.0f, 0.0f)))
                {
                    spectrumData = spectrumSolver.Calculate(
                        grid,
                        config.energyMin,
                        config.energyMax,
                        config.energySamples,
                        g,
                        barrierIndex,
                        config.tolerance
                    );

                    waveFunctionData.Clear();
                }

                ImGui::EndDisabled();

                if (!spectrumData.Empty())
                {
                    ImGui::Text("Точек графика: %d", spectrumData.Size());
                    ImGui::Text("Найдено уровней: %d", static_cast<int>(spectrumData.eigenvalues.size()));

                    ImGui::BeginChild("Собственные энергии", ImVec2(0.0f, 150.0f), true);

                    for (int i = 0; i < static_cast<int>(spectrumData.eigenvalues.size()); ++i)
                    {
                        const double epsilon = spectrumData.eigenvalues[i];
                        const double physicalEnergy = system.ToPhysicalEnergy(epsilon);

                        ImGui::Text("n = %d", i);
                        ImGui::Text("epsilon = %.10f", epsilon);
                        ImGui::Text("E = %.6e Дж", physicalEnergy);
                        ImGui::Separator();
                    }

                    ImGui::EndChild();
                }
            }

            if (ImGui::CollapsingHeader("Волновая функция", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Номер состояния n");
                ImGui::SetNextItemWidth(-1.0f);

                if (ImGui::InputInt("##stateIndex", &config.stateIndex))
                    waveFunctionData.Clear();

                if (config.stateIndex < 0)
                    config.stateIndex = 0;

                const bool stateValid =
                    config.stateIndex >= 0 &&
                    config.stateIndex < static_cast<int>(spectrumData.eigenvalues.size());

                if (spectrumData.eigenvalues.empty())
                {
                    ImGui::TextWrapped("Сначала рассчитайте спектр.");
                }
                else if (!stateValid)
                {
                    ImGui::TextColored(
                        ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                        "Допустимый номер состояния: от 0 до %d.",
                        static_cast<int>(spectrumData.eigenvalues.size()) - 1
                    );
                }

                ImGui::BeginDisabled(!gridValid || !stateValid);

                if (ImGui::Button("Рассчитать волновую функцию", ImVec2(-1.0f, 0.0f)))
                {
                    const double eigenvalue = spectrumData.eigenvalues[config.stateIndex];

                    waveFunctionData = waveFunctionSolver.Calculate(
                        grid,
                        eigenvalue,
                        g,
                        barrierIndex,
                        config.stateIndex
                    );
                }

                ImGui::EndDisabled();

                if (!waveFunctionData.Empty())
                {
                    const double physicalEnergy = system.ToPhysicalEnergy(waveFunctionData.energy);

                    ImGui::Text("Состояние n = %d", waveFunctionData.stateIndex);
                    ImGui::Text("epsilon_n = %.10f", waveFunctionData.energy);
                    ImGui::Text("E_n = %.6e Дж", physicalEnergy);
                }
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Выход", ImVec2(-1.0f, 0.0f)))
            exitRequested = true;

        ImGui::EndChild();
        ImGui::SameLine();

        if (systemValid && rangeValid)
        {
            for (int i = 0; i < plotPointCount; ++i)
            {
                const double t = static_cast<double>(i) / static_cast<double>(plotPointCount - 1);
                xi[i] = config.xiLeft + t * (config.xiRight - config.xiLeft);
                potential[i] = system.GetDimensionlessPotential(xi[i]);
            }
        }

        const bool selectedStateValid =
            config.stateIndex >= 0 &&
            config.stateIndex < static_cast<int>(spectrumData.eigenvalues.size());

        double selectedEpsilon = 0.0;

        if (selectedStateValid)
            selectedEpsilon = spectrumData.eigenvalues[config.stateIndex];

        ImGui::BeginChild("Графики", ImVec2(0.0f, 0.0f), true);

        if (ImGui::BeginTabBar("Вкладки графиков"))
        {
            if (ImGui::BeginTabItem("Потенциал"))
            {
                if (!systemValid)
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Исправьте физические параметры.");
                }
                else if (!rangeValid)
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Правая граница должна быть больше левой.");
                }
                else if (ImPlot::BeginPlot("Безразмерный потенциал", ImVec2(-1.0f, -1.0f)))
                {
                    ImPlot::SetupAxes("xi", "V(xi)");
                    ImPlot::PlotLine("V(xi) = xi^2 / 2", xi.data(), potential.data(), plotPointCount);

                    if (alpha >= config.xiLeft && alpha <= config.xiRight && std::abs(g) > 1.0e-14)
                    {
                        const double maxAbsXi = std::max(std::abs(config.xiLeft), std::abs(config.xiRight));
                        const double maxPotential = 0.5 * maxAbsXi * maxAbsXi;
                        double barrierX[2] = {alpha, alpha};
                        double barrierY[2] = {0.0, maxPotential};

                        ImPlot::PlotLine("Дельта-барьер", barrierX, barrierY, 2);
                    }

                    if (selectedStateValid)
                    {
                        double energyX[2] = {config.xiLeft, config.xiRight};
                        double energyY[2] = {selectedEpsilon, selectedEpsilon};

                        ImPlot::PlotLine("Выбранный уровень энергии", energyX, energyY, 2);
                    }

                    ImPlot::EndPlot();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Фаза"))
            {
                if (spectrumData.Empty())
                {
                    ImGui::TextWrapped("Нажмите \"Рассчитать спектр\", чтобы построить фазовую лесенку.");
                }
                else if (ImPlot::BeginPlot("Фаза на правой границе", ImVec2(-1.0f, -1.0f)))
                {
                    ImPlot::SetupAxes("epsilon", "theta_R / pi");

                    ImPlot::PlotLine(
                        "theta_R / pi",
                        spectrumData.energies.data(),
                        spectrumData.phasesOverPi.data(),
                        spectrumData.Size()
                    );

                    for (int i = 0; i < static_cast<int>(spectrumData.eigenvalues.size()); ++i)
                    {
                        const double eigenvalue = spectrumData.eigenvalues[i];
                        ImPlot::TagX(eigenvalue, ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "E%d", i);
                    }

                    ImPlot::EndPlot();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Волновая функция"))
            {
                if (waveFunctionData.Empty())
                {
                    ImGui::TextWrapped("Сначала рассчитайте спектр и выбранную волновую функцию.");
                }
                else if (ImPlot::BeginPlot("Стационарная волновая функция", ImVec2(-1.0f, -1.0f)))
                {
                    ImPlot::SetupAxes("xi", "psi_n(xi)");

                    ImPlot::PlotLine(
                        "psi_n",
                        waveFunctionData.x.data(),
                        waveFunctionData.psi.data(),
                        waveFunctionData.Size()
                    );

                    ImPlot::EndPlot();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndChild();

        if (exitRequested)
        {
            ImGui::OpenPopup("Подтверждение выхода");
            exitRequested = false;
        }

        if (ImGui::BeginPopupModal("Подтверждение выхода", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Закрыть программу?");
            ImGui::Spacing();

            if (ImGui::Button("Выйти", ImVec2(120.0f, 0.0f)))
            {
                running = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Отмена", ImVec2(120.0f, 0.0f)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
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