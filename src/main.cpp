#include <raylib.h>

#include <imgui.h>
#include <implot.h>
#include <rlImGui.h>

#include <cmath>
#include <vector>

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

    // Инициализация Dear ImGui через raylib.
    rlImGuiSetup(true);

    // ImPlot требует свой отдельный контекст.
    ImPlot::CreateContext();

    const int pointCount = 500;

    std::vector<double> x(pointCount);
    std::vector<double> y(pointCount);

    for (int i = 0; i < pointCount; ++i)
    {
        x[i] =
            -5.0 +
            10.0 *
            static_cast<double>(i) /
            static_cast<double>(pointCount - 1);

        y[i] = x[i] * x[i];
    }

    while (!WindowShouldClose())
    {
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
                static_cast<float>(GetScreenWidth()),
                static_cast<float>(GetScreenHeight())
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

        ImGui::Text(
            "Harmonic oscillator test"
        );

        ImGui::Separator();

        if (ImPlot::BeginPlot(
            "Potential U(x) = x^2",
            ImVec2(-1, -1)
        ))
        {
            ImPlot::SetupAxes(
                "x",
                "U(x)"
            );

            ImPlot::PlotLine(
                "U(x)",
                x.data(),
                y.data(),
                pointCount
            );

            ImPlot::EndPlot();
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