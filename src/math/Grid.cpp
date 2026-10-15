#include "Grid.h"

#include <cmath>

Grid::Grid() : left_(0.0), right_(0.0), step_(0.0), pointCount_(0), valid_(false)
{
}

Grid::Grid(double left, double right, int pointCount) : Grid()
{
    Build(left, right, pointCount);
}

bool Grid::Build(double left, double right, int pointCount)
{
    valid_ = false;
    points_.clear();

    if (right <= left || pointCount < 2)
        return false;

    left_ = left;
    right_ = right;
    pointCount_ = pointCount;
    step_ = (right_ - left_) / static_cast<double>(pointCount_ - 1);

    points_.resize(pointCount_);

    for (int i = 0; i < pointCount_; ++i)
        points_[i] = left_ + static_cast<double>(i) * step_;

    valid_ = true;
    return true;
}

bool Grid::IsValid() const
{
    return valid_;
}

bool Grid::Contains(double value) const
{
    return valid_ && value >= left_ && value <= right_;
}

double Grid::GetLeft() const
{
    return left_;
}

double Grid::GetRight() const
{
    return right_;
}

double Grid::GetStep() const
{
    return step_;
}

int Grid::GetPointCount() const
{
    return pointCount_;
}

double Grid::GetPoint(int index) const
{
    if (!valid_ || index < 0 || index >= pointCount_)
        return 0.0;

    return points_[index];
}

const std::vector<double>& Grid::GetPoints() const
{
    return points_;
}

int Grid::FindNearestIndex(double value) const
{
    if (!Contains(value))
        return -1;

    const double relativePosition = (value - left_) / step_;
    int index = static_cast<int>(std::round(relativePosition));

    if (index < 0)
        index = 0;

    if (index >= pointCount_)
        index = pointCount_ - 1;

    return index;
}