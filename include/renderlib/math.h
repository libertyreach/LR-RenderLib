#pragma once

#include <cmath>
#include <string>
#include <sstream>
#include <functional>

namespace renderlib {

template <typename T>
T clamp(T value, T min, T max)
{
    return (value < min ? min : (value > max ? max : value));
}

template <class T>
class Quaternion;

template <typename T, size_t N>
struct Vector
{
    std::array<T, N> vector;

    Vector(std::array<T, N> const& vect)
    {
        for (int i = 0; i < N; i++)
        {
            vector[i] = i < vect.size() ? vect[i] : 0;
        }
    }

    Vector()
    {
        for (int i = 0; i < N; i++) { vector[i] = 0; }
    }

    Vector(T val)
    {
        for (int i = 0; i < N; i++) { vector[i] = val; }
    }

    Vector(const Vector& vect)
    {
        for (int i = 0; i < N; i++) { vector[i] = vect[i]; }
    }

    Vector(T x, T y)
    {
        vector[0] = x;
        vector[1] = y;
    }

    Vector(T x, T y, T z)
    {
        vector[0] = x;
        vector[1] = y;
        vector[2] = z;
    }

    // Static Helpers

    static Vector Identity()
    {
        Vector vect;
        for (int i = 0; i < N; i++) { vect[i] = 1; }
        return vect;
    }

    static Vector Zero()
    {
        Vector vect;
        return vect;
    }

    static Vector<T, 3> Right() { return Vector(1, 0, 0); }

    static Vector<T, 3> Left() { return Vector(-1, 0, 0); }

    static Vector<T, 3> Top() { return Vector(0, 1, 0); }

    static Vector<T, 3> Bottom() { return Vector(0, -1, 0); }

    static Vector<T, 3> Face() { return Vector(0, 0, 1); }

    static Vector<T, 3> Back() { return Vector(0, 0, -1); }

    static Vector<T, 3> Right(T value) { return Vector(value, 0, 0); }

    static Vector<T, 3> Left(T value) { return Vector(-value, 0, 0); }

    static Vector<T, 3> Top(T value) { return Vector(0, value, 0); }

    static Vector<T, 3> Bottom(T value) { return Vector(0, -value, 0); }

    static Vector<T, 3> Face(T value) { return Vector(0, 0, value); }

    static Vector<T, 3> Back(T value) { return Vector(0, 0, -value); }

    // Basic accessors
    T x() { return vector[0]; }

    T y() { return vector[1]; }

    T z() { return vector[2]; }

    Vector<T, 2> xy() { return Vector<T, 2>(vector[0], vector[1]); }

    Vector<T, 2> xz() { return Vector<T, 2>(vector[0], vector[2]); }

    Vector<T, 2> yz() { return Vector<T, 2>(vector[1], vector[2]); }

    Vector<T, 3> xyz() { return Vector<T, 3>(vector[0], vector[1], vector[2]); }

    // Basic functions

    // Set all elements to zero

    Vector copy() const
    {
        Vector copy = Vector();
        for (int i = 0; i < N; i++) { copy[i] = vector[i]; }
        return copy;
    }

    // In-place set all members to 0
    void zero()
    {
        for (int i = 0; i < N; i++) { vector[i] = 0; }
    }

    // Update all values of vector based on index and value
    void update(std::function<T(int, T)> lambda)
    {
        for (int i = 0; i < N; i++) { vector[i] = lambda(i, vector[i]); }
    }

    size_t getSize() const { return N; }

    std::string toString() const
    {
        std::string out = "[";
        for (int i = 0; i < N; i++)
        {
            if (i > 0) out += ", ";
            out += std::to_string(vector[i]);
        }
        out += "]";
        return out;
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector& vect)
    {
        out << vect.toString();
        return out;
    }

    operator std::string() const
    {
        std::ostringstream out;
        out << *this;
        return out.str();
    }

    T distance(const Vector& other) const
    {
        T dist = 0;
        for (int i = 0; i < N; i++) { dist += std::abs(other[i] - vector[i]); }
        return dist;
    }

    T magnitude() const
    {
        T sum = 0;
        for (int i = 0; i < N; i++) { sum += vector[i] * vector[i]; }
        return sqrt(sum);
    }

    T magnitudeSq() const
    {
        T mag = magnitude();
        return mag * mag;
    }

    Vector normalized() const
    {
        Vector copy = *this;
        T mag = copy.magnitude();
        if (mag != 0)
        {
            for (int i = 0; i < N; i++) { copy[i] /= mag; }
        }
        return copy;
    }

    void normalize()
    {
        T mag = magnitude();
        if (mag != 0)
        {
            for (int i = 0; i < N; i++) { vector[i] /= mag; }
        }
    }

    Vector rounded(float step = 1.0f) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++)
        {
            // copy[i] = std::round(copy[i]);
            copy[i] = (float)std::floor(copy[i] * step + 0.5f) / step;
        }
        return copy;
    }

    Vector abs() const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] = std::abs(copy[i]); }
        return copy;
    }

    T sum() const
    {
        T total = 0;
        for (int i = 0; i < N; i++) { total += vector[i]; }
        return total;
    }

    void round()
    {
        for (int i = 0; i < N; i++) { vector[i] = std::round(vector[i]); }
    }

    Vector floor() const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] = std::floor(copy[i]); }
        return copy;
    }

    Vector ceil() const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] = std::ceil(copy[i]); }
        return copy;
    }

    Vector sign() const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++)
        {
            copy[i] = copy[i] > 0 ? 1 : (copy[i] < 0 ? -1 : 0);
        }
        return copy;
    }

    T dot(Vector vect) const
    {
        T sum = 0;
        for (int i = 0; i < N; i++) { sum += vector[i] * vect[i]; }
        return sum;
    }

    T angleBetween(Vector vect) const
    {
        return atan2(cross(vect).dot(Vector::Face()), dot(vect));
    }

    Quaternion<T> rotationBetween(Vector vect)
    {
        float dotProd = dot(vect);
        float k = sqrt(magnitudeSq() * vect.magnitudeSq());

        if (dotProd / k == -1)
        {
            // TODO: fix this
            return Quaternion<T>(0, orth().normalized());
        }

        Quaternion q = Quaternion(
            0.5f * 3.141592653589793238462 * (dotProd + k), cross(vect));
        q.normalize();

        return q;
    }

    // Compute an orthaganol vector
    Vector orth() const
    {
        if (dot(Vector::Top()) == 0.0) { return cross(Vector::Top()); }
        else if (dot(Vector::Right()) == 0.0) { return cross(Vector::Right()); }
        else { return cross(Vector::Face()); }
    }

    // Return the vector such that V.prod(V.inverse()) = V.Identity();
    Vector inverse() const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] = 1 / vector[i]; };
        return copy;
    }


    Vector<T, 3> cross(Vector vect) const
    {
        return Vector<T, 3>(
            vector[1] * vect[2] - vector[2] * vect[1],
            -(vector[0] * vect[2] - vector[2] * vect[0]),
            vector[0] * vect[1] - vector[1] * vect[0]);
    }

    // element-wise multiplication
    Vector prod(Vector vect) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] *= vect[i]; };
        return copy;
    }

    // element-wise division
    Vector div(Vector vect) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] /= vect[i]; };
        return copy;
    }

    // Return vect projected onto this vector
    Vector proj(Vector vect) const
    {
        return vect * (dot(vect) / vect.dot(vect));
    }

    // Cast vector from type T to V
    template <class V>
    Vector<V, N> cast() const
    {
        Vector<V, N> copy = Vector<V, N>();
        for (int i = 0; i < N; i++) { copy[i] = static_cast<V>(vector[i]); }
        return copy;
    }

    template <size_t toSize>
    Vector<T, toSize> resize(T defaultValue = 0.0) const
    {
        Vector<T, toSize> copy = Vector<T, toSize>();
        for (int i = 0; i < toSize; i++)
        {
            if (i < N) { copy.vector[i] = vector[i]; }
            else { copy.vector[i] = defaultValue; }
        }
        return copy;
    }

    T min() const
    {
        bool hasBest = false;
        T best;
        for (int i = 0; i < N; i++)
        {
            if (!hasBest || vector[i] < best)
            {
                hasBest = true;
                best = vector[i];
            }
        }
        return best;
    }

    T max() const
    {
        bool hasBest = false;
        T best;
        for (int i = 0; i < N; i++)
        {
            if (!hasBest || vector[i] > best)
            {
                hasBest = true;
                best = vector[i];
            }
        }
        return best;
    }

    Vector bounded(Vector by) const
    {
        auto out = copy();
        float maxDim = max();
        float byMaxDim = by.max();
        float scale = maxDim / byMaxDim;

        if (scale > 1) { out *= 1 / scale; }

        return out;
    }

    Vector fill(Vector to) const
    {
        auto out = copy();
        float scale = max() / to.max();
        return out * (1 / scale);
    }

    void clamp(T low, T high)
    {
        for (int i = 0; i < N; i++) { vector[i] = clamp(vector[i], low, high); }
    }

    void clamp(Vector low, Vector high)
    {
        for (int i = 0; i < N; i++)
        {
            vector[i] = clamp(vector[i], low[i], high[i]);
        }
    }

    bool approxEqual(Vector other, T epsilon) const
    {
        for (int i = 0; i < N; i++)
        {
            if (other[i] < vector[i] - epsilon ||
                other[i] > vector[i] + epsilon)
            {
                return false;
            }
        }
        return false;
    }

    // Operator Overloads

    T operator[](const int i) const { return vector[i]; }

    T& operator[](const int i) { return vector[i]; }

    bool operator==(const Vector& vect) const
    {
        for (int i = 0; i < N; i++)
        {
            if (vector[i] != vect[i]) return false;
        }
        return true;
    }

    bool operator!=(const Vector& vect) const
    {
        for (int i = 0; i < N; i++)
        {
            if (vector[i] != vect[i]) return true;
        }
        return false;
    }

    bool operator<(const Vector& vect) const noexcept
    {
        T magA = 0.0f;
        T magB = 0.0f;
        for (int i = 0; i < N; i++)
        {
            magA += vector[i] * vector[i];
            magB += vect[i] * vect[i];
        }
        magA = sqrt(magA);
        magB = sqrt(magB);

        return magA < magB;
    }

    Vector operator+(const Vector& vect) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] += vect[i]; }
        return copy;
    }

    Vector operator-(const Vector& vect) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] -= vect[i]; }
        return copy;
    }

    Vector operator*(T scalar) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] *= scalar; }
        return copy;
    }

    Vector operator/(T scalar) const
    {
        Vector copy = *this;
        for (int i = 0; i < N; i++) { copy[i] /= scalar; }
        return copy;
    }

    Vector& operator+=(const Vector& vect)
    {
        for (int i = 0; i < N; i++) { vector[i] += vect[i]; }
        return *this;
    }

    Vector& operator-=(const Vector& vect)
    {
        for (int i = 0; i < N; i++) { vector[i] -= vect[i]; }
        return *this;
    }

    Vector& operator*=(T scalar)
    {
        for (int i = 0; i < N; i++) { vector[i] *= scalar; }
        return *this;
    }

    Vector& operator/=(T scalar)
    {
        for (int i = 0; i < N; i++) { vector[i] /= scalar; }
        return *this;
    }

    std::size_t hash() const
    {
        std::size_t seed = N;
        for (auto& i : vector)
        {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <class T>
class Quaternion : public Vector<T, 4>
{
private:
public:
    Quaternion() { Quaternion(0, Vector<T, 3>(0, 0, 0)); }

    Quaternion(T rotation, Vector<T, 3> axisOfRotation)
    {
        this->vector[0] = cos(0.5 * rotation);
        this->vector[1] = axisOfRotation[0] * sin(0.5 * rotation);
        this->vector[2] = axisOfRotation[1] * sin(0.5 * rotation);
        this->vector[3] = axisOfRotation[2] * sin(0.5 * rotation);
    }

    Quaternion(T w, T i, T j, T k)
    {
        this->vector[0] = w;
        this->vector[1] = i;
        this->vector[2] = j;
        this->vector[3] = k;
    }

    Quaternion(Vector<T, 3> euler) : Quaternion(euler[0], euler[1], euler[2]) {}

    Quaternion(T roll, T pitch, T yaw)
    {
        T cy = cos(yaw * 0.5);
        T sy = sin(yaw * 0.5);
        T cp = cos(pitch * 0.5);
        T sp = sin(pitch * 0.5);
        T cr = cos(roll * 0.5);
        T sr = sin(roll * 0.5);

        this->vector[0] = cr * cp * cy + sr * sp * sy;
        this->vector[1] = sr * cp * cy - cr * sp * sy;
        this->vector[2] = cr * sp * cy + sr * cp * sy;
        this->vector[3] = cr * cp * sy - sr * sp * cy;
    }

    Quaternion operator*(const Quaternion& quat) const
    {
        return Quaternion(
            this->vector[0] * quat[0] - this->vector[1] * quat[1] -
                this->vector[2] * quat[2] - this->vector[3] * quat[3],
            this->vector[0] * quat[1] + this->vector[1] * quat[0] +
                this->vector[2] * quat[3] - this->vector[3] * quat[2],
            this->vector[0] * quat[2] - this->vector[1] * quat[3] +
                this->vector[2] * quat[0] + this->vector[3] * quat[1],
            this->vector[0] * quat[3] + this->vector[1] * quat[2] -
                this->vector[2] * quat[1] + this->vector[3] * quat[0]);
    }

    Quaternion operator*(const Vector<T, 3>& vec) const
    {
        return *this * Quaternion(0, vec[0], vec[1], vec[2]);
    }

    Quaternion operator*(T scalar) const
    {
        return Quaternion(
            this->vector[0] * scalar, this->vector[1] * scalar,
            this->vector[2] * scalar, this->vector[3] * scalar);
    }

    Quaternion operator/(T scalar) const
    {
        return Quaternion(
            this->vector[0] / scalar, this->vector[1] / scalar,
            this->vector[2] / scalar, this->vector[3] / scalar);
    }

    T magnitude() const
    {
        return sqrt(
            this->vector[0] * this->vector[0] +
            this->vector[1] * this->vector[1] +
            this->vector[2] * this->vector[2] +
            this->vector[3] * this->vector[3]);
    }

    Quaternion conjugate() const
    {
        return Quaternion(
            this->vector[0], -this->vector[1], -this->vector[2],
            -this->vector[3]);
    }

    Quaternion inverse() const { return conjugate() / magnitude(); }

    Vector<T, 3> imaginary() const
    {
        return Vector<T, 3>(this->vector[1], this->vector[2], this->vector[3]);
    }

    T real() const { return this->vector[0]; }

    Vector<T, 3> rotatePoint(Vector<T, 3> point) const
    {
        return ((*this * point) * conjugate()).imaginary();
    }

    Quaternion<T> normalized() const
    {
        T mag = magnitude();
        return (*this) / mag;
    }

    // Spherical Linear Interpolation (Slerp)
    Quaternion<T> slerp(const Quaternion<T>& target, T t) const
    {
        T dotProduct = dot(target);

        // If the dot product is negative, use the opposite direction
        if (dotProduct < 0.0f) { return slerp(-target, t); }

        // If the quaternions are very close, use linear interpolation
        if (dotProduct > 0.9995f)
        {
            return normalized() * (1 - t) + target.normalize() * t;
        }

        dotProduct = clamp(dotProduct, -1.0f, 1.0f);
        T theta_0 = acos(dotProduct);  // angle between the two quaternions
        T theta = theta_0 * t;  // angle at the interpolation point
        Quaternion<T> targetNorm =
            (target - (*this) * dotProduct)
                .normalize();  // perpendicular quaternion

        return (*this) * cos(theta) + targetNorm * sin(theta);
    }

    // Computes the dot product between two quaternions
    T dot(const Quaternion<T>& quat) const
    {
        return this->vector[0] * quat[0] + this->vector[1] * quat[1] +
               this->vector[2] * quat[2] + this->vector[3] * quat[3];
    }

    Vector<T, 3> eulerAngles() const
    {
        T a = this->vector[0];
        T b = this->vector[1];
        T c = this->vector[2];
        T d = this->vector[3];

        // Roll
        T roll = atan2(2 * (a * b + c * d), 1 - 2 * (b * b + c * c));

        // Pitch
        T pitch = asin(2 * (a * c - d * b));

        // Clamp pitch to the range [-pi/2, pi/2]
        if (pitch > PI / 2) pitch = PI / 2;
        else if (pitch < -PI / 2) pitch = -PI / 2;

        // Yaw
        T yaw = atan2(2 * (a * d + b * c), 1 - 2 * (c * c + d * d));

        return Vector<T, 3>(roll, pitch, yaw);
    }
};

template <size_t size, class T>
inline Vector<T, size> operator*(T scalar, const Vector<T, size>& rhs)
{
    Vector<T, size> copy = Vector<T, size>(rhs);
    return copy * scalar;
}

template <size_t size, class T>
inline Vector<T, size> normalize(Vector<T, size> vector)
{
    return vector.normalized();
}

template <size_t size, class T>
inline Vector<T, size> cross(Vector<T, size> a, Vector<T, size> b)
{
    return a.cross(b);
}

template <size_t size, class T>
inline T dot(Vector<T, size> a, Vector<T, size> b)
{
    return a.dot(b);
}

template <size_t size, class T>
inline T dot2(Vector<T, size> a)
{
    return a.dot(a);
}

template <size_t Size, typename T>
inline T distance(Vector<T, Size> a, Vector<T, Size> b)
{
    return (b - a).magnitude();
}


template <class T>
Quaternion<T> operator*(const Quaternion<T>& lhs, const Quaternion<T>& rhs)
{
    return Quaternion(
        lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2] - lhs[3] * rhs[3],
        lhs[0] * rhs[1] + lhs[1] * rhs[0] + lhs[2] * rhs[3] - lhs[3] * rhs[2],
        lhs[0] * rhs[2] - lhs[1] * rhs[3] + lhs[2] * rhs[0] + lhs[3] * rhs[1],
        lhs[0] * rhs[3] + lhs[1] * rhs[2] + lhs[2] * rhs[1] + lhs[3] * rhs[0]);
}

template <class T>
Quaternion<T> operator*(T scalar, const Quaternion<T>& rhs)
{
    return Quaternion(
        rhs[0] * scalar, rhs[1] * scalar, rhs[2] * scalar, rhs[3] * scalar);
}

template <class T>
Quaternion<T> operator*(Vector<T, 3> vec, const Quaternion<T>& rhs)
{
    return Quaternion(0, vec[0], vec[1], vec[2]) * rhs;
}

template <typename T>
struct Mat4
{
    std::array<T, 16> m{};

    static Mat4 identity()
    {
        Mat4 r;
        r.m = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        return r;
    }

    static Mat4 translate(Vector<T, 3> t)
    {
        Mat4 r = identity();
        r.m[12] = t[0];
        r.m[13] = t[1];
        r.m[14] = t[2];
        return r;
    }

    // Euler angles in degrees, applied X then Y then Z (R = Rz * Ry * Rx).
    static Mat4 rotateEuler(Vector<T, 3> degrees)
    {
        // Note: not named DEG2RAD — raylib's raymath.h defines that as a macro.
        float toRad = 3.14159265358979323846f / 180.0f;
        float cx = std::cos(degrees[0] * toRad);
        float sx = std::sin(degrees[0] * toRad);
        float cy = std::cos(degrees[1] * toRad);
        float sy = std::sin(degrees[1] * toRad);
        float cz = std::cos(degrees[2] * toRad);
        float sz = std::sin(degrees[2] * toRad);

        Mat4 rx = identity();
        rx.m[5] = cx;
        rx.m[6] = sx;
        rx.m[9] = -sx;
        rx.m[10] = cx;
        Mat4 ry = identity();
        ry.m[0] = cy;
        ry.m[2] = -sy;
        ry.m[8] = sy;
        ry.m[10] = cy;
        Mat4 rz = identity();
        rz.m[0] = cz;
        rz.m[1] = sz;
        rz.m[4] = -sz;
        rz.m[5] = cz;

        return rz * ry * rx;
    }

    // The translation column, i.e. this transform's origin in world space.
    Vector<T, 3> translation() const { return {m[12], m[13], m[14]}; }

    friend Mat4 operator*(Mat4 const& a, Mat4 const& b)
    {
        Mat4 r;
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
