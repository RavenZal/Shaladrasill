#pragma once

struct vec3f
{
    float x, y, z;
    vec3f() : x(0), y(0), z(0) {}
    vec3f(float _v) : x(_v), y(_v), z(_v) {}
    vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};
