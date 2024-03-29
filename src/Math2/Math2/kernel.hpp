#pragma once

// #define __MATH_DISABLE_NORMALIZATION_CHECK 1

#if __DEBUG
#if __MATH_DISABLE_NORMALIZATION_CHECK
#define __MATH_NORMALIZATION_CHECK 0
#else
#define __MATH_NORMALIZATION_CHECK 1
#endif
#else
#define __MATH_NORMALIZATION_CHECK 0
#endif

inline int8 Math::equals(const float32 p_left, const float32 p_right)
{
    return fabsf(p_left - p_right) <= Limits::tol_f;
};

inline int8 Math::nequals(const float32 p_left, const float32 p_right)
{
    return fabsf(p_left - p_right) > Limits::tol_f;
};

inline int8 Math::lower_eq(const float32 p_left, const float32 p_right)
{
    return (p_left - p_right) <= Limits::tol_f;
};

inline int8 Math::lower(const float32 p_left, const float32 p_right)
{
    return (p_left - p_right) < Limits::tol_f;
};

inline int8 Math::greater_eq(const float32 p_left, const float32 p_right)
{
    return (p_left - p_right) >= Limits::tol_f;
};

inline int8 Math::greater(const float32 p_left, const float32 p_right)
{
    return (p_left - p_right) > Limits::tol_f;
};

inline int16 Math::sign(const float32 p_value)
{
    if (p_value <= -Limits::tol_f)
    {
        return -1;
    }
    else
    {
        return 1;
    }
};

inline float32 Math::clamp_f32(const float32 p_value, const float32 p_left, const float32 p_right)
{
    if (p_value >= (p_right + Limits::tol_f))
    {
        return p_right;
    }
    else if (p_value <= (p_left + Limits::tol_f))
    {
        return p_left;
    }
    return p_value;
};

inline uint32 Math::clamp_ui32(const uint32 p_value, const uint32 p_left, const uint32 p_right)
{
    if (p_value >= p_right)
    {
        return p_right;
    }
    else if (p_value <= p_left)
    {
        return p_left;
    }
    return p_value;
};

inline uint8 Math::sRGB_to_linear_uint8(const uint8 p_sRGB)
{
    float l_s = (float)p_sRGB / (float)uint8_max;
    if (l_s <= 0.04045f)
    {
        return (uint8)nearbyintf((l_s / 12.92f) * uint8_max);
    }
    else
    {
        return (uint8)nearbyintf(powf((l_s + 0.055f) / 1.055f, 2.4f) * uint8_max);
    }
};

inline uint8 Math::linear_to_sRGB_uint8(const uint8 p_linear)
{
    float l_linear = (float)p_linear / (float)uint8_max;
    if (l_linear <= 0.0031308f)
    {
        return (uint8)nearbyintf((l_linear * 12.92f) * uint8_max);
    }
    else
    {
        return (uint8)nearbyintf((1.055f * (powf(l_linear, 1.0f / 2.4f)) - 0.055f) * uint8_max);
    }
};

inline float32 Math::sRGB_to_linear_float32(const float32 p_sRGB)
{
    if (p_sRGB <= 0.04045f)
    {
        return (p_sRGB / 12.92f);
    }
    else
    {
        return powf((p_sRGB + 0.055f) / 1.055f, 2.4f);
    }
};

inline float32 Math::linear_to_sRGB_float32(const float32 p_linear)
{
    if (p_linear <= 0.0031308f)
    {
        return (p_linear * 12.92f);
    }
    else
    {
        return (1.055f * (powf(p_linear, 1.0f / 2.4f)) - 0.055f);
    }
};

inline int8 v2f::operator==(const v2f& p_other)
{
    return Math::equals(this->Points[0], p_other.Points[0]) && Math::equals(this->Points[1], p_other.Points[1]);
};

inline v2f v2f::operator*(const v2f& p_other)
{
    return v2f{
        this->x * p_other.x,
        this->y * p_other.y,
    };
};

inline int8 v3f_assert_is_normalized(const v3f& p_vec)
{
    return Math::equals(p_vec.length(), 1.0f);
};

inline int8 v3fn_assert_is_normalized(const v3fn& p_vec)
{
    return v3f_assert_is_normalized(p_vec.vec3);
};

inline int8 v4f_assert_is_normalized(const v4f& p_vec)
{
    return Math::equals(p_vec.length(), 1.0f);
};
inline int8 v4fn_assert_is_normalized(const v4fn& p_vec)
{
    return v4f_assert_is_normalized(p_vec.Vec4);
};

inline int8 quat_assert_is_normalized(const quat& p_quat)
{
    return v4f_assert_is_normalized(p_quat.Points);
};

inline v3f v3f::operator+(const v3f& p_other) const
{
    return v3f{this->x + p_other.x, this->y + p_other.y, this->z + p_other.z};
};

inline v3f v3f::operator*(const float32 p_other) const
{
    return v3f{this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other};
};

inline v3f v3f::operator*(const v3f& p_other) const
{
    return v3f{this->x * p_other.x, this->y * p_other.y, this->z * p_other.z};
};

inline v3f v3f::operator-(const v3f& p_other) const
{
    return v3f{this->x - p_other.x, this->y - p_other.y, this->z - p_other.z};
};

inline int8 v3f::operator==(const v3f& p_other) const
{
    return Math::equals(this->Points[0], p_other.Points[0]) && Math::equals(this->Points[1], p_other.Points[1]) && Math::equals(this->Points[2], p_other.Points[2]);
};

inline int8 v3f::operator!=(const v3f& p_other) const
{
    return Math::nequals(this->Points[0], p_other.Points[0]) || Math::nequals(this->Points[1], p_other.Points[1]) || Math::nequals(this->Points[2], p_other.Points[2]);
};

inline float32& v3f::operator[](const uint8 p_index)
{
    return this->Points[p_index];
};

inline float32 v3f::dot(const v3f& p_other) const
{
    v3f l_mul = this->operator*(p_other);
    return l_mul.x + l_mul.y + l_mul.z;
};

inline v3f v3f::cross(const v3f& p_other) const
{
    return v3f{(this->Points[1] * p_other.Points[2]) - (this->Points[2] * p_other.Points[1]), (this->Points[2] * p_other.Points[0]) - (this->Points[0] * p_other.Points[2]),
               (this->Points[0] * p_other.Points[1]) - (this->Points[1] * p_other.Points[0])};
};

inline float32 v3f::length() const
{
    v3f l_squared = v3f{this->x * this->x, this->y * this->y, this->z * this->z};
    return sqrtf(l_squared.x + l_squared.y + l_squared.z);
};

inline v3fn v3f::normalize() const
{
    v3fn l_return;
    l_return.vec3 = this->operator*(1.0f / this->length());
    return l_return;
};

inline v3f v3f::inv() const
{
    return v3f{1.0f / this->x, 1.0f / this->y, 1.0f / this->z};
};

inline v3f v3f::project(const v3f& p_projected_on) const
{
    return this->normalize().project(p_projected_on.normalize());
};

inline v3f v3f::project(const v3fn& p_projected_on) const
{
    return this->normalize().project(p_projected_on);
};

inline float32 v3f::distance(const v3f& p_end) const
{
    return this->operator-(p_end).length();
};

inline float32 v3f::angle_unsigned(const v3f& p_end) const
{
    return acosf(this->dot(p_end) / (this->length() * p_end.length()));
};

inline float32 v3f::angle_unsigned(const v3fn& p_end) const
{
#if __MATH_NORMALIZATION_CHECK
    v3fn_assert_is_normalized(p_end);
#endif
    return acosf(this->dot(p_end.vec3) / (this->length()));
};

inline int8 v3f::anglesign(const v3f& p_end, const v3f& p_ref_axis) const
{
    float32 l_dot = this->cross(p_end).dot(p_ref_axis);
    return l_dot >= Limits::tol_f ? 1 : -1;
};

inline int8 v3f::anglesign(const v3f& p_end, const v3fn& p_ref_axis) const
{
    return this->anglesign(p_end, p_ref_axis.vec3);
};

inline int8 v3f::anglesign(const v3fn& p_end, const v3f& p_ref_axis) const
{
    return this->anglesign(p_end.vec3, p_ref_axis);
};

inline int8 v3f::anglesign(const v3fn& p_end, const v3fn& p_ref_axis) const
{
    return this->anglesign(p_end.vec3, p_ref_axis.vec3);
};

inline v3f v3f::rotate(const quat& p_rotation) const
{
    return (p_rotation * quat::build_v3f_f(*this, 0.0f) * p_rotation.inv()).Vec3s.Vec * this->length();
};

inline quat v3f::euler_to_quat() const
{
    v3f l_cos = v3f{cosf(this->Points[0] * 0.5f), cosf(this->Points[1] * 0.5f), cosf(this->Points[2] * 0.5f)};
    v3f l_sin = v3f{sinf(this->Points[0] * 0.5f), sinf(this->Points[1] * 0.5f), sinf(this->Points[2] * 0.5f)};

    quat l_return = quat{l_sin.x * l_cos.y * l_cos.z - l_cos.x * l_sin.y * l_sin.z, l_cos.x * l_sin.y * l_cos.z + l_sin.x * l_cos.y * l_sin.z,
                         l_cos.x * l_cos.y * l_sin.z - l_sin.x * l_sin.y * l_cos.z, l_cos.x * l_cos.y * l_cos.z + l_sin.x * l_sin.y * l_sin.z};
#if __MATH_NORMALIZATION_CHECK
    assert_true(quat_assert_is_normalized(l_return));
#endif
    return l_return;
};

inline quat v3f::from_to(const v3f& p_to) const
{
    return this->normalize().from_to(p_to.normalize());
};

inline quat v3f::from_to(const v3fn& p_to) const
{
    return this->normalize().from_to(p_to);
};

inline v3f v3fn::operator+(const v3f& p_other) const
{
    return this->vec3.operator+(p_other);
};

inline v3f v3fn::operator+(const v3fn& p_other) const
{
    return this->vec3.operator+(p_other.vec3);
};

inline v3f v3fn::operator*(const float32 p_other) const
{
    return this->vec3.operator*(p_other);
};

inline v3f v3fn::operator*(const v3f& p_other) const
{
    return this->vec3.operator*(p_other);
};

inline v3f v3fn::operator*(const v3fn& p_other) const
{
    return this->vec3.operator*(p_other.vec3);
};

inline v3f v3fn::operator-(const v3f& p_other) const
{
    return this->vec3.operator-(p_other);
};

inline v3f v3fn::operator-(const v3fn& p_other) const
{
    return this->vec3.operator-(p_other.vec3);
};

inline int8 v3fn::operator==(const v3f& p_other) const
{
    return this->vec3.operator==(p_other);
};

inline int8 v3fn::operator==(const v3fn& p_other) const
{
    return this->vec3.operator==(p_other.vec3);
};

inline int8 v3fn::operator!=(const v3f& p_other) const
{
    return this->vec3.operator!=(p_other);
};

inline int8 v3fn::operator!=(const v3fn& p_other) const
{
    return this->vec3.operator!=(p_other.vec3);
};

inline float32& v3fn::operator[](const uint8 p_index)
{
    return this->Points[p_index];
};

inline float32 v3fn::dot(const v3f& p_other) const
{
    return this->vec3.dot(p_other);
};

inline float32 v3fn::dot(const v3fn& p_other) const
{
    return this->vec3.dot(p_other.vec3);
};

inline v3f v3fn::cross(const v3f& p_other) const
{
    return this->vec3.cross(p_other);
};

inline v3f v3fn::cross(const v3fn& p_other) const
{
    return this->vec3.cross(p_other.vec3);
};

inline float32 v3fn::length() const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(*this));
#endif
    return 1.0f;
};

inline v3f v3fn::project(const v3fn& p_projected_on) const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(*this));
    assert_true(v3fn_assert_is_normalized(p_projected_on));
#endif

    return p_projected_on * this->dot(p_projected_on);
};

inline float32 v3fn::angle_unsigned(const v3f& p_end) const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(*this));
#endif

    return acosf(this->dot(p_end) / (p_end.length()));
};

inline float32 v3fn::angle_unsigned(const v3fn& p_end) const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(*this));
    assert_true(v3fn_assert_is_normalized(p_end));
#endif

    return acosf(this->dot(p_end));
};

inline int8 v3fn::anglesign(const v3f& p_end, const v3f& p_ref_axis) const
{
    return this->vec3.anglesign(p_end, p_ref_axis);
};

inline int8 v3fn::anglesign(const v3f& p_end, const v3fn& p_ref_axis) const
{
    return this->vec3.anglesign(p_end, p_ref_axis);
};

inline int8 v3fn::anglesign(const v3fn& p_end, const v3f& p_ref_axis) const
{
    return this->vec3.anglesign(p_end, p_ref_axis);
};

inline int8 v3fn::anglesign(const v3fn& p_end, const v3fn& p_ref_axis) const
{
    return this->vec3.anglesign(p_end, p_ref_axis);
};

inline v3fn v3fn::rotate(const quat& p_rotation) const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(*this));
#endif

    v3fn l_return = (p_rotation * quat::build_v3fn_f(*this, 0.0f) * p_rotation.inv()).Vec3ns.Vec;

#if __MATH_NORMALIZATION_CHECK
    assert_true(v3fn_assert_is_normalized(l_return));
#endif
    return l_return;
};

inline quat v3fn::euler_to_quat() const
{
    return this->vec3.euler_to_quat();
};

inline quat v3fn::from_to(const v3fn& p_to) const
{
#if __MATH_NORMALIZATION_CHECK
    v3fn_assert_is_normalized(*this);
    v3fn_assert_is_normalized(p_to);
#endif

    float32 l_costtheta = this->dot(p_to);
    if (l_costtheta >= Math_const::one_f - Limits::tol_f)
    {
        return quat_const::IDENTITY;
    }

    {
        v3f l_rotation_axis;

        if (l_costtheta < -Math_const::one_f + Limits::tol_f)
        {
            l_rotation_axis = v3f_const::FORWARD.cross(*this);
            if (l_rotation_axis.length() < Limits::tol_f)
            {
                l_rotation_axis = v3f_const::RIGHT.cross(*this);
            }
            v3fn l_rotation_axis_normalized = l_rotation_axis.normalize();
            return quat::rotate_around(l_rotation_axis_normalized, Math_const::PI);
        }
    }

    v3f l_rotation_axis = this->cross(p_to);
    float32 l_s = sqrtf((Math_const::one_f + l_costtheta) * 2.0f);
    float32 l_invs = 1.0f / l_s;

    quat l_return = quat{l_rotation_axis.x * l_invs, l_rotation_axis.y * l_invs, l_rotation_axis.z * l_invs, l_s * 0.5f};
#if __MATH_NORMALIZATION_CHECK
    assert_true(quat_assert_is_normalized(l_return));
#endif
    return l_return;
};

inline quat v3fn::from_to(const v3f& p_to) const
{
    return this->from_to(p_to.normalize());
};

inline int8 v3ui::operator==(const v3ui& p_other) const
{
    return Slice<v3ui>::build_asint8_memory_singleelement(this).compare(Slice<v3ui>::build_asint8_memory_singleelement(&p_other));
};

inline int8 v3ui::operator!=(const v3ui& p_other) const
{
    return !Slice<v3ui>::build_asint8_memory_singleelement(this).compare(Slice<v3ui>::build_asint8_memory_singleelement(&p_other));
};

inline v4f v4f::operator+(const v4f& p_other) const
{
    return v4f{this->x + p_other.x, this->y + p_other.y, this->z + p_other.z, this->w + p_other.w};
};

inline int8 v4f::operator==(const v4f& p_other) const
{
    return Math::equals(this->Points[0], p_other.Points[0]) && Math::equals(this->Points[1], p_other.Points[1]) && Math::equals(this->Points[2], p_other.Points[2]) &&
           Math::equals(this->Points[3], p_other.Points[3]);
};

inline int8 v4f::operator!=(const v4f& p_other) const
{
    return Math::nequals(this->Points[0], p_other.Points[0]) || Math::nequals(this->Points[1], p_other.Points[1]) || Math::nequals(this->Points[2], p_other.Points[2]) ||
           Math::nequals(this->Points[3], p_other.Points[3]);
};

inline v4f v4f::operator*(const float32 p_other) const
{
    return v4f{this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other, this->Points[3] * p_other};
};

inline v4f v4f::operator*(const v4f& p_other) const
{
    return v4f{this->x * p_other.x, this->y * p_other.y, this->z * p_other.z, this->w * p_other.w};
};

inline float32& v4f::operator[](const uint8 p_index)
{
    return this->Points[p_index];
};

inline float32 v4f::length() const
{
    v4f l_squared = v4f{this->x * this->x, this->y * this->y, this->z * this->z, this->w * this->w};
    return sqrtf(l_squared.x + l_squared.y + l_squared.z + l_squared.w);
};

inline v4fn v4f::normalize() const
{
    v4fn l_return;
    l_return.Vec4 = this->operator*(1.0f / this->length());
    return l_return;
};

inline v4f v4f::sRGB_to_linear() const
{
    return v4f{Math::sRGB_to_linear_float32(this->x), Math::sRGB_to_linear_float32(this->y), Math::sRGB_to_linear_float32(this->z), Math::sRGB_to_linear_float32(this->w)};
};

inline v4f v4f::linear_to_sRGB() const
{
    return v4f{Math::linear_to_sRGB_float32(this->x), Math::linear_to_sRGB_float32(this->y), Math::linear_to_sRGB_float32(this->z), Math::linear_to_sRGB_float32(this->w)};
};

inline v4ui8 v4f::to_uint8_color() const
{
    return v4ui8{(uint8)nearbyintf(this->x * uint8_max), (uint8)nearbyintf(this->y * uint8_max), (uint8)nearbyintf(this->z * uint8_max), (uint8)nearbyintf(this->w * uint8_max)};
};

inline v4f v4fn::operator+(const v4fn& p_other) const
{
    return this->Vec4.operator+(p_other.Vec4);
};

inline int8 v4fn::operator==(const v4fn& p_other) const
{
    return this->Vec4.operator==(p_other.Vec4);
};

inline int8 v4fn::operator!=(const v4fn& p_other) const
{
    return this->Vec4.operator!=(p_other.Vec4);
};

inline v4f v4fn::operator*(const float32 p_other) const
{
    return this->Vec4.operator*(p_other);
};

inline v4f v4fn::operator*(const v4fn& p_other) const
{
    return this->Vec4.operator*(p_other.Vec4);
};

inline float32& v4fn::operator[](const uint8 p_index)
{
    return this->Points[p_index];
};

inline float32 v4fn::length() const
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v4fn_assert_is_normalized(*this));
#endif
    return 1.0f;
};

inline int8 v4ui8::operator==(const v4ui8& p_other) const
{
    return (this->Points[0] == p_other.Points[0]) && (this->Points[1] == p_other.Points[1]) && (this->Points[2] == p_other.Points[2]) && (this->Points[3] == p_other.Points[3]);
};

inline v4ui8 v4ui8::sRGB_to_linear() const
{
    return v4ui8{
        Math::sRGB_to_linear_uint8(this->x),
        Math::sRGB_to_linear_uint8(this->y),
        Math::sRGB_to_linear_uint8(this->z),
        Math::sRGB_to_linear_uint8(this->w),
    };
};

inline v4ui8 v4ui8::linear_to_sRGB() const
{
    return v4ui8{Math::linear_to_sRGB_uint8(this->x), Math::linear_to_sRGB_uint8(this->y), Math::linear_to_sRGB_uint8(this->z), Math::linear_to_sRGB_uint8(this->w)};
};

inline v4f v4ui8::to_color_f() const
{
    return v4f{(float)this->x / uint8_max, (float)this->y / uint8_max, (float)this->z / uint8_max, (float)this->w / uint8_max};
};

inline quat quat::rotate_around(const v3fn& p_axis, const float32 p_angle)
{
    quat l_return = quat::build_v3f_f(p_axis * sinf(p_angle * 0.5f), cosf(p_angle * 0.5f));
#if __MATH_NORMALIZATION_CHECK
    assert_true(quat_assert_is_normalized(l_return));
#endif
    return l_return;
};

inline int8 quat::operator==(const quat& p_other) const
{
    return this->Points == p_other.Points;
};

inline int8 quat::operator!=(const quat& p_other) const
{
    return this->Points != p_other.Points;
};

inline quat quat::operator*(const quat& p_other) const
{
    return quat::build_v3f_f(((this->Vec3s.Vec * p_other.w) + (p_other.Vec3s.Vec * this->w)) + this->Vec3s.Vec.cross(p_other.Vec3s.Vec), (this->w * p_other.w) - this->Vec3s.Vec.dot(p_other.Vec3s.Vec))
        .normalize();
};

inline quat quat::normalize() const
{
    return quat::build_v4fn(this->Points.normalize());
};

inline quat quat::inv() const
{
    return quat::build_v3f_f(this->Vec3s.Vec * -1.0f, this->Vec3s.Scal);
};

inline quat quat::cross(const quat& p_other) const
{
    v3fn l_rotated_left = v3f_const::FORWARD.rotate(*this);
    v3fn l_rotated_right = v3f_const::FORWARD.rotate(p_other);
    quat l_return = quat::rotate_around(l_rotated_left.cross(l_rotated_right).normalize(), 0.0f);
#if __MATH_NORMALIZATION_CHECK
    assert_true(quat_assert_is_normalized(l_return));
#endif
    return l_return;
};

inline m33f quat::to_axis() const
{

    float32 l_qxx = this->x * this->x;
    float32 l_qxy = this->x * this->y;
    float32 l_qxz = this->x * this->z;
    float32 l_qxw = this->x * this->w;

    float32 l_qyy = this->y * this->y;
    float32 l_qyz = this->y * this->z;
    float32 l_qyw = this->y * this->w;

    float32 l_qzz = this->z * this->z;
    float32 l_qzw = this->z * this->w;

    m33f l_return;
    // RIGHT
    l_return.Col0 = v3f{1 - (2 * l_qyy) - (2 * l_qzz), (2 * l_qxy) + (2 * l_qzw), (2 * l_qxz) - (2 * l_qyw)};

    // UP
    l_return.Col1 = v3f{(2 * l_qxy) - (2 * l_qzw), 1 - (2 * l_qxx) - (2 * l_qzz), (2 * l_qyz) + (2 * l_qxw)};

    // Forward
    l_return.Col2 = v3f{(2 * l_qxz) + (2 * l_qyw), (2 * l_qyz) - (2 * l_qxw), 1 - (2 * l_qxx) - (2 * l_qyy)};

    l_return.Col0 = l_return.Col0.normalize().vec3;
    l_return.Col1 = l_return.Col1.normalize().vec3;
    l_return.Col2 = l_return.Col2.normalize().vec3;

    return l_return;
};

inline v3f quat::euler() const
{
    v3f l_return;

    // pitch
    float32 l_sinp = 2.0f * (this->y * this->z + this->w * this->x);
    float32 l_cosp = this->w * this->w - this->x * this->x - this->y * this->y + this->z * this->z;

    if (Math::equals(l_sinp, Math_const::zero_f) && Math::equals(l_cosp, Math_const::zero_f))
    {
        l_return.Points[0] = 2.0f * atan2f(this->x, this->w);
    }
    else
    {
        l_return.Points[0] = atan2f(l_sinp, l_cosp);
    }

    // yaw
    l_return.Points[1] = asinf(Math::clamp_f32(-2.0f * (this->x * this->z - this->w * this->y), -1.0f, 1.0f));

    // roll
    l_return.Points[2] = atan2f(2.0f * (this->x * this->y + this->w * this->z), this->w * this->w + this->x * this->x - this->y * this->y - this->z * this->z);

    return l_return;
};

inline quat m33f::to_rotation() const
{
    const v3f& l_right = this->Right;
    const v3f& l_up = this->Up;
    const v3f& l_forward = this->Forward;

    // We calculate the four square roots and get the higher one.
    float32 qxDiag = fmaxf(1 + l_right.x - l_up.y - l_forward.z, 0.0f);
    float32 qyDiag = fmaxf(1 + l_up.y - l_right.x - l_forward.z, 0.0f);
    float32 qzDiag = fmaxf(1 + l_forward.z - l_right.x - l_up.y, 0.0f);
    float32 qwDiag = fmaxf(1 + l_right.x + l_up.y + l_forward.z, 0.0f);

    int l_diagonalIndex = 0;
    float32 l_biggestDiagonalValue = qxDiag;
    if (qyDiag > l_biggestDiagonalValue)
    {
        l_biggestDiagonalValue = qyDiag;
        l_diagonalIndex = 1;
    }
    if (qzDiag > l_biggestDiagonalValue)
    {
        l_biggestDiagonalValue = qzDiag;
        l_diagonalIndex = 2;
    }
    if (qwDiag > l_biggestDiagonalValue)
    {
        l_biggestDiagonalValue = qwDiag;
        l_diagonalIndex = 3;
    }

    l_biggestDiagonalValue = 0.5f * sqrtf(l_biggestDiagonalValue);
    float32 mult = 1 / (4.0f * l_biggestDiagonalValue);

    switch (l_diagonalIndex)
    {
    case 0:
    {
        return quat{l_biggestDiagonalValue, (l_right.y + l_up.x) * mult, (l_forward.x + l_right.z) * mult, (l_up.z - l_forward.y) * mult}.normalize();
    }
    break;
    case 1:
    {
        return quat{(l_right.y + l_up.x) * mult, l_biggestDiagonalValue, (l_up.z + l_forward.y) * mult, (l_forward.x - l_right.z) * mult}.normalize();
    }
    break;
    case 2:
    {
        return quat{(l_forward.x + l_right.z) * mult, (l_up.z + l_forward.y) * mult, l_biggestDiagonalValue, (l_right.y - l_up.x) * mult}.normalize();
    }
    break;
    case 3:
    {
        return quat{(l_up.z - l_forward.y) * mult, (l_forward.x - l_right.z) * mult, (l_right.y - l_up.x) * mult, l_biggestDiagonalValue}.normalize();
    }
    break;
    }

    return quat{0.0f, 0.0f, 0.0f, 1.0f};
}

#define mat_foreach_element_begin(Dimension)                                                                                                                                                           \
    for (uint8 p_column_index = 0; p_column_index < Dimension; p_column_index++)                                                                                                                       \
    {                                                                                                                                                                                                  \
        for (uint8 p_line_index = 0; p_line_index < Dimension; p_line_index++)                                                                                                                         \
        {

#define mat_foreach_element_end()                                                                                                                                                                      \
    }                                                                                                                                                                                                  \
    }

inline int8 m33f::operator==(const m33f& p_other) const
{
    mat_foreach_element_begin(3) if (!Math::equals(this->Points2D[p_column_index].Points[p_line_index], p_other.Points2D[p_column_index].Points[p_line_index]))
    {
        return 0;
    }
    mat_foreach_element_end();
    return 1;
};

inline v3f& m33f::operator[](const uint8 p_index)
{
    return this->Points2D[p_index];
};

inline m33f m33f::lookat(const v3f& p_origin, const v3f& p_target, const v3fn& p_up)
{
#if __MATH_NORMALIZATION_CHECK
    v3fn_assert_is_normalized(p_up);
#endif

    m33f l_return = m33f_const::IDENTITY;

    l_return.Forward = (p_target - p_origin).normalize().vec3;
    l_return.Right = p_up.cross(l_return.Forward).normalize().vec3;        // (Up x Forward  is Right)
    l_return.Up = l_return.Forward.cross(l_return.Right).normalize().vec3; // (Forward x Right is Up)

    return l_return;
};

inline float32 m44f::mul_line_column(const m44f& p_left, const m44f& p_right, const uint8 p_column_index, const uint8 p_line_index)
{
    float32 l_return = 0;
    for (int16 i = 0; i < 4; i++)
    {
        l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points2D[p_column_index].Points[i]);
    }
    return l_return;
};

inline float32 m44f::mul_line_vec(const m44f& p_left, const v4f& p_right, const uint8 p_line_index)
{
    float32 l_return = Math_const::zero_f;
    for (int16 i = 0; i < 4; i++)
    {
        l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points[i]);
    }
    return l_return;
};

inline m44f m44f::operator*(const m44f& p_other) const
{
    m44f l_return;
    mat_foreach_element_begin(4) l_return[p_column_index][p_line_index] = mul_line_column(*this, p_other, p_column_index, p_line_index);
    mat_foreach_element_end() return l_return;
};

inline v4f m44f::operator*(const v4f& p_other) const
{
    v4f l_return;
    for (int8 p_line_index = 0; p_line_index < 4; p_line_index++)
    {
        l_return[p_line_index] = mul_line_vec(*this, p_other, p_line_index);
    }
    return l_return;
};

inline m44f m44f::operator*(const float32 p_other) const
{
    m44f l_return;
    mat_foreach_element_begin(4) l_return[p_column_index][p_line_index] = this->Points2D[p_column_index].Points[p_line_index] * p_other;
    mat_foreach_element_end() return l_return;
};

inline m44f m44f::operator+(const float32 p_other) const
{
    m44f l_return;
    mat_foreach_element_begin(4) l_return[p_column_index][p_line_index] = this->Points2D[p_column_index].Points[p_line_index] + p_other;
    mat_foreach_element_end() return l_return;
};

inline int8 m44f::operator==(const m44f& p_other) const
{
    mat_foreach_element_begin(4) if (!Math::equals(this->Points2D[p_column_index].Points[p_line_index], p_other.Points2D[p_column_index].Points[p_line_index]))
    {
        return 0;
    };
    mat_foreach_element_end();

    return 1;
};

inline v4f& m44f::operator[](const uint8 p_index)
{
    return this->Points2D[p_index];
};

inline const v4f& m44f::operator[](const uint8 p_index) const
{
    return this->Points2D[p_index];
};

inline float32 m44f::det(const uint8 p_column_index, const uint8 p_line_index) const
{
    m33f l_matDet;
    uint8 l_matDet_column_counter = 0;
    uint8 l_matDet_line_counter = 0;

    for (uint8 l_column_index = 0; l_column_index < 4; l_column_index++)
    {
        if (l_column_index != p_column_index)
        {
            l_matDet_line_counter = 0;
            for (uint8 l_line_index = 0; l_line_index < 4; l_line_index++)
            {
                if (l_line_index != p_line_index)
                {
                    l_matDet.Points2D[l_matDet_column_counter].Points[l_matDet_line_counter] = this->Points2D[l_column_index].Points[l_line_index];
                    l_matDet_line_counter += 1;
                }
            }
            l_matDet_column_counter += 1;
        }
    }

    return (l_matDet[0][0] * ((l_matDet[1][1] * l_matDet[2][2]) - (l_matDet[1][2] * l_matDet[2][1]))) + (l_matDet[1][0] * ((l_matDet[2][1] * l_matDet[0][2]) - (l_matDet[2][2] * l_matDet[0][1]))) +
           (l_matDet[2][0] * ((l_matDet[0][1] * l_matDet[1][2]) - (l_matDet[0][2] * l_matDet[1][1])));
};

inline m44f m44f::inv() const
{
    m44f l_return;
    float32 l_det = (this->Points2D[0].Points[0] * this->det(0, 0)) - (this->Points2D[0].Points[1] * this->det(0, 1)) + (this->Points2D[0].Points[2] * this->det(0, 2)) -
                    (this->Points2D[0].Points[3] * this->det(0, 3));

    {
        l_return._00 = this->det(0, 0);
        l_return._01 = -this->det(1, 0);
        l_return._02 = this->det(2, 0);
        l_return._03 = -this->det(3, 0);
        l_return._10 = -this->det(0, 1);
        l_return._11 = this->det(1, 1);
        l_return._12 = -this->det(2, 1);
        l_return._13 = this->det(3, 1);
        l_return._20 = this->det(0, 2);
        l_return._21 = -this->det(1, 2);
        l_return._22 = this->det(2, 2);
        l_return._23 = -this->det(3, 2);
        l_return._30 = -this->det(0, 3);
        l_return._31 = this->det(1, 3);
        l_return._32 = -this->det(2, 3);
        l_return._33 = this->det(3, 3);
    }

    return l_return * (1.0f / l_det);
};

inline const v3f& m44f::get_translation() const
{
    return this->Col3.Vec3;
};

inline m44f m44f::build_translation(const v3f& p_translation)
{
    m44f l_return = m44f_const::IDENTITY;
    l_return.Col3.Vec3 = p_translation;
    return l_return;
};

inline m44f m44f::build_rotation(const m33f& p_axis)
{
    return m44f::build_columns(v4f::build_v3f_s(p_axis.Points2D[0], 0.0f), v4f::build_v3f_s(p_axis.Points2D[1], 0.0f), v4f::build_v3f_s(p_axis.Points2D[2], 0.0f), v4f{0.0f, 0.0f, 0.0f, 1.0f});
};

inline m44f m44f::build_rotation(const v3fn& p_right, const v3fn& p_up, const v3fn& p_forward)
{
    return m44f::build_columns(v4f::build_v3fn_s(p_right, 0.0f), v4f::build_v3fn_s(p_up, 0.0f), v4f::build_v3fn_s(p_forward, 0.0f), v4f{0.0f, 0.0f, 0.0f, 1.0f});
};

inline m44f m44f::build_scale(const v3f& p_scale)
{
    return m44f::build_columns(v4f{p_scale.x, 0.0f, 0.0f, 0.0f}, v4f{0.0f, p_scale.y, 0.0f, 0.0f}, v4f{0.0f, 0.0f, p_scale.z, 0.0f}, v4f{0.0f, 0.0f, 0.0f, 1.0f});
};

inline m44f m44f::trs(const m44f& p_translation, const m44f& p_rotation, const m44f& p_scale)
{
#if __MATH_NORMALIZATION_CHECK
    assert_true(v4f_assert_is_normalized(p_rotation.Col0));
    assert_true(v4f_assert_is_normalized(p_rotation.Col1));
    assert_true(v4f_assert_is_normalized(p_rotation.Col2));
    assert_true(v4f_assert_is_normalized(p_rotation.Col3));
#endif
    return (p_translation * p_rotation) * p_scale;
};

inline m44f m44f::trs(const v3f& p_translation, const m33f& p_axis, const v3f& p_scale)
{
    return m44f::trs(m44f::build_translation(p_translation), m44f::build_rotation(p_axis), m44f::build_scale(p_scale));
};

inline m44f m44f::lookat_rotation(const v3f& p_origin, const v3f& p_target, const v3f& p_up)
{
    m44f l_return = m44f_const::IDENTITY;

    l_return.Forward.Vec3 = (p_target - p_origin).normalize().vec3;
    l_return.Right.Vec3 = p_up.cross(l_return.Forward.Vec3).normalize().vec3;
    l_return.Up.Vec3 = l_return.Forward.Vec3.cross(l_return.Right.Vec3).normalize().vec3;

    return l_return;
};

inline m44f m44f::lookat_rotation(const v3f& p_origin, const v3f& p_target, const v3fn& p_up)
{
    return m44f::lookat_rotation(p_origin, p_target, p_up.vec3);
};

inline m44f m44f::view(const v3f& p_world_position, const v3f& p_forward, const v3f& p_up)
{
    return m44f::view(p_world_position, p_forward, p_up.normalize());
};

inline m44f m44f::view(const v3f& p_world_position, const v3f& p_forward, const v3fn& p_up)
{
#if __MATH_NORMALIZATION_CHECK
    v3fn_assert_is_normalized(p_up);
#endif
    v3f l_target = p_forward;
    l_target = p_world_position + l_target;
    v3f l_up = p_up * -1.0f;
    return m44f::trs(m44f::build_translation(p_world_position), m44f::lookat_rotation(p_world_position, l_target, l_up), m44f::build_scale(v3f_const::ONE.vec3)).inv();
};

inline m44f m44f::perspective(const float32 p_fov, const float32 p_aspect, const float32 p_near, const float32 p_far)
{
    m44f l_return;
    float32 l_halfTan = tanf(p_fov / 2.0f);

    l_return._00 = 1.0f / (p_aspect * l_halfTan);
    l_return._01 = 0.0f;
    l_return._02 = 0.0f;
    l_return._03 = 0.0f;

    l_return._10 = 0.0f;
    l_return._11 = 1.0f / l_halfTan;
    l_return._12 = 0.0f;
    l_return._13 = 0.0f;

    l_return._20 = 0.0f;
    l_return._21 = 0.0f;
    l_return._22 = (p_far + p_near) / (p_far - p_near);
    l_return._23 = 1.0f;

    l_return._30 = 0.0f;
    l_return._31 = 0.0f;
    l_return._32 = (-2.0f * p_far * p_near) / (p_far - p_near);
    l_return._33 = 0.0f;

    return l_return;
};

inline int8 transform::operator==(const transform& p_other)
{
    return (this->position == p_other.position) & (this->rotation == p_other.rotation) & (this->scale == p_other.scale);
};