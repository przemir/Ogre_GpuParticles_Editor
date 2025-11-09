/*
 * File:   Range.cpp
 * Author: Przemysław Bągard
 * Created: 2017-03-10
 *
 */
#include "Range.h"
#include <limits>
#include <cmath>
#include <cstdlib>

const Geometry::Range Geometry::Range::UnitRange(0.0f, 1.0f);
const Geometry::Range Geometry::Range::EmptyRange = Geometry::Range::createEmptyRange();

void Geometry::Range::extend(float value)
{
    if(value < _min) {
        _min = value;
    }
    if(value > _max) {
        _max = value;
    }
}

void Geometry::Range::extend(const Geometry::Range& other)
{
    if(other.isEmpty()) {
        return;
    }
    extend(other.getMin());
    extend(other.getMax());
}

Geometry::Range Geometry::Range::common(const Geometry::Range& a, const Geometry::Range& b)
{
    if(a.isEmpty()) {
        return a;
    }
    if(b.isEmpty()) {
        return b;
    }
    Geometry::Range result;
    result._min = a._min < b._min ? b._min : a._min;
    result._max = a._max > b._max ? b._max : a._max;
    return result;
}

bool Geometry::Range::areRangesOverlapping(const Geometry::Range& range1, const Geometry::Range& range2)
{
    if(range1.isEmpty() || range2.isEmpty()) {
        return false;
    }

    if(range1._min > range2._max) {
        return false;
    }

    if(range1._max < range2._min) {
        return false;
    }

    return true;
}

bool Geometry::Range::areRangesOverlappingIgnoreBounds(const Geometry::Range& range1, const Geometry::Range& range2)
{
    if(range1.isEmpty() || range2.isEmpty()) {
        return false;
    }

    if(range1._min >= range2._max) {
        return false;
    }

    if(range1._max <= range2._min) {
        return false;
    }

    return true;
}

Geometry::Range Geometry::Range::createEmptyRange()
{
    Range range;
    range._min = INFINITY;
    range._max = -INFINITY;
    return range;
}


// iRange

void Geometry::iRange::extend(int value)
{
    if(value < _min) {
        _min = value;
    }
    if(value > _max) {
        _max = value;
    }
}

Geometry::iRange Geometry::iRange::common(const Geometry::iRange& a, const Geometry::iRange& b)
{
    if(a.isEmpty()) {
        return a;
    }
    if(b.isEmpty()) {
        return b;
    }
    Geometry::iRange result;
    result._min = a._min < b._min ? b._min : a._min;
    result._max = a._max > b._max ? b._max : a._max;
    return result;
}

bool Geometry::iRange::areRangesOverlapping(const Geometry::iRange& range1, const Geometry::iRange& range2)
{
    if(range1.isEmpty() || range2.isEmpty()) {
        return false;
    }

    if(range1._min > range2._max) {
        return false;
    }

    if(range1._max < range2._min) {
        return false;
    }

    return true;
}

Geometry::iRange Geometry::iRange::createEmptyRange()
{
    iRange range;
    range._min = 1;
    range._max = -1;
    return range;
}
