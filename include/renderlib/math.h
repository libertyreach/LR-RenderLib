#pragma once

#include <cmath>

#include "types.h"

namespace renderlib {

// Column-major 4x4 matrix using the OpenGL memory layout:
//   element(row, col) == m[col * 4 + row]
// This matches the order raylib's rlMultMatrixf() expects, so the raw data can
// be handed to the backend without transposing (same as MatrixToFloat).
struct mat4
{
    std::array<float, 16> m{};

    static mat4 identity()
    {
        mat4 r;
        r.m = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        return r;
    }

    static mat4 translate(vec3 t)
    {
        mat4 r = identity();
        r.m[12] = t[0];
        r.m[13] = t[1];
        r.m[14] = t[2];
        return r;
    }

    // Euler angles in degrees, applied X then Y then Z (R = Rz * Ry * Rx).
    static mat4 rotateEuler(vec3 degrees)
    {
        // Note: not named DEG2RAD — raylib's raymath.h defines that as a macro.
        constexpr float toRad = 3.14159265358979323846f / 180.0f;
        float cx = std::cos(degrees[0] * toRad);
        float sx = std::sin(degrees[0] * toRad);
        float cy = std::cos(degrees[1] * toRad);
        float sy = std::sin(degrees[1] * toRad);
        float cz = std::cos(degrees[2] * toRad);
        float sz = std::sin(degrees[2] * toRad);

        mat4 rx = identity();
        rx.m[5] = cx;  rx.m[6] = sx;  rx.m[9] = -sx;  rx.m[10] = cx;
        mat4 ry = identity();
        ry.m[0] = cy;  ry.m[2] = -sy;  ry.m[8] = sy;  ry.m[10] = cy;
        mat4 rz = identity();
        rz.m[0] = cz;  rz.m[1] = sz;  rz.m[4] = -sz;  rz.m[5] = cz;

        return rz * ry * rx;
    }

    // The translation column, i.e. this transform's origin in world space.
    vec3 translation() const { return {m[12], m[13], m[14]}; }

    friend mat4 operator*(mat4 const& a, mat4 const& b)
    {
        mat4 r;
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                }
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }
};

}  // namespace renderlib
