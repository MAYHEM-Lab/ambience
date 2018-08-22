//
// Created by fatih on 8/22/18.
//

#pragma once


#include <stdlib.h>

struct vec3
{
    float x, y, z;
};

vec3 operator+(const vec3& a, const vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3 operator-(const vec3& a, const vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3 operator*(const vec3& a, float x)
{
    return { a.x * x, a.y * x, a.z * x };
}

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

class fake_accel
{
public:
    fake_accel(vec3 begin, vec3 end) : m_begin{begin}, m_end{end} {}

    vec3 sample()
    {
        auto x = (rand() % 1024) / 1024.f;
        return lerp(m_begin, m_end, x);
    }

private:
    vec3 m_begin;
    vec3 m_end;
};

