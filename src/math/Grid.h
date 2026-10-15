#pragma once

#include <vector>

class Grid
{
public:
    Grid();
    Grid(double left, double right, int pointCount);

    bool Build(double left, double right, int pointCount);
    bool IsValid() const;
    bool Contains(double value) const;

    double GetLeft() const;
    double GetRight() const;
    double GetStep() const;
    int GetPointCount() const;

    double GetPoint(int index) const;
    const std::vector<double>& GetPoints() const;

    int FindNearestIndex(double value) const;

private:
    double left_;
    double right_;
    double step_;
    int pointCount_;
    bool valid_;

    std::vector<double> points_;
};