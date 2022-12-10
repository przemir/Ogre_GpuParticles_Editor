/*
 * File:   Range.h
 * Author: Przemysław Bągard
 * Created: 2017-03-10
 *
 */
#ifndef RANGE_H
#define RANGE_H

#include <assert.h>

namespace Geometry {

/// Float range
struct Range
{
    Range()
        : _min(0.0f)
        , _max(0.0f)
    {
    }

    Range(float value)
        : _min(value)
        , _max(value)
    {
    }

    Range(float min, float max)
        : _min(min)
        , _max(max)
    {
        if(min > max) {
            _min = max;
            _max = min;
        }
    }

    inline float getMin() const
    {
        assert(!isEmpty());
        return _min;
    }

    inline float getMax() const
    {
        assert(!isEmpty());
        return _max;
    }

    inline float lerp(float percent) const
    {
        assert(!isEmpty());
        return _min + (_max-_min)*percent;
    }

    inline bool inside(float value) const
    {
        return value >= _min && value <= _max;
    }

    inline bool insideWithEpsilon(float value, float epsilon = 0.001f) const
    {
        if(isEmpty()) {
            return false;
        }
        return value >= _min-epsilon && value <= _max+epsilon;
    }

    inline float clamp(float value) const
    {
        if(value < _min) {
            return _min;
        }
        if(value > _max) {
            return _max;
        }
        return value;
    }

    inline bool isEmpty() const { return _min > _max; }

    /// if value is outside of range, range is extended
    void extend(float value);
    void extend(const Range& other);

    /// Common part of 'a' and 'b'
    static Range common(const Range& a, const Range& b);

    /// [1; 2] and [2; 3] are overlapping
    static bool areRangesOverlapping(const Range& range1, const Range& range2);

    /// This meand that [1; 2] and [2; 3] will not be overlapping.
    static bool areRangesOverlappingIgnoreBounds(const Range& range1, const Range& range2);

    static Range createEmptyRange();

    static const Range UnitRange;
    static const Range EmptyRange;

private:
    float _min;
    float _max;

//    inline void ensureIsSorted();
    //{
    //    if(_min > _max) {
    //        float temp = _min;
    //        _min = _max;
    //        _max = temp;
    //    }
    //}
};

/// Integer range
struct iRange
{
    iRange(int value)
        : _min(value)
        , _max(value)
    {
    }

    iRange(int min, int max)
        : _min(min)
        , _max(max)
    {
        if(min > max) {
            _min = max;
            _max = min;
        }
    }

    inline int getMin() const
    {
        assert(!isEmpty());
        return _min;
    }

    inline int getMax() const
    {
        assert(!isEmpty());
        return _max;
    }

    inline bool inside(int value) const
    {
        return value >= _min && value <= _max;
    }

    inline int clamp(int value) const
    {
        if(value < _min) {
            return _min;
        }
        if(value > _max) {
            return _max;
        }
        return value;
    }

    inline bool isEmpty() const { return _min > _max; }

    /// if value is outside of range, range is extended
    void extend(int value);

    /// Common part of 'a' and 'b'
    static iRange common(const iRange& a, const iRange& b);

    static bool areRangesOverlapping(const iRange& range1, const iRange& range2);

    static iRange createEmptyRange();

private:
    iRange()
        : _min(0)
        , _max(0)
    {}

    int _min;
    int _max;
};

}

#endif
