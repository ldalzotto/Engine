
#include "Math2/math.hpp"

inline void math_tests()
{
    assert_true((v3f{1.0f, 2.0f, 3.0f} + v3f{1.0f, 2.0f, 3.0f}) == v3f{2.0f, 4.0f, 6.0f});
    assert_true((v3f{1.0f, 2.0f, 3.0f} - v3f{1.0f, 2.0f, 3.0f}) == v3f_const::ZERO.vec3);
    assert_true((v3f{1.0f, 2.0f, 3.0f} * v3f{1.0f, 2.0f, 3.0f}) == v3f{1.0f, 4.0f, 9.0f});
    assert_true(v3f{1.0f, 2.0f, 3.0f}.dot(v3f{1.0f, 2.0f, 3.0f}) == 14.0f);
    assert_true(v3f{1.0f, 2.0f, 3.0f}.cross(v3f{1.0f, 2.0f, 3.0f}) == v3f{0.0f, 0.0f, 0.0f});
    assert_true(v3f{1.0f, 2.0f, 3.0f}.length() == 3.74165750f);
    assert_true(v3f{1.0f, 2.0f, 3.0f}.normalize().vec3 == v3f{0.267261237f, 0.534522474f, 0.801783681f});
    assert_true(v3f{1.0f, 2.0f, 3.0f}.project(v3f{0.3f, 0.1f, 0.9f}) == v3f{0.281945944f, 0.0939819813f, 0.845837712f});
    assert_true(v3f{1.0f, 2.0f, 3.0f}.distance(v3f{0.3f, 0.1f, 0.9f}) == 2.91719031f);

    assert_true(Math::equals(v3f{1.0f, 2.0f, 3.0f}.angle_unsigned(v3f{0.3f, 0.1f, 0.9f}), 0.458921432f));
    float32 l_angle_unsigned_comparison = 0.458921432f;
    assert_true(Math::equals(v3f{1.0f, 2.0f, 3.0f}.normalize().angle_unsigned(v3f{0.3f, 0.1f, 0.9f}), l_angle_unsigned_comparison));
    assert_true(Math::equals(v3f{1.0f, 2.0f, 3.0f}.angle_unsigned(v3f{0.3f, 0.1f, 0.9f}.normalize()), l_angle_unsigned_comparison));
    assert_true(Math::equals(v3f{1.0f, 2.0f, 3.0f}.normalize().angle_unsigned(v3f{0.3f, 0.1f, 0.9f}.normalize()), l_angle_unsigned_comparison));

    assert_true(v3f{1.0f, 2.0f, 3.0f}.anglesign(v3f{0.3f, 0.1f, 0.9f}, v3f_const::FORWARD) == -1.0f);

    assert_true(v3f{1.0f, 2.0f, 3.0f}.rotate(quat::rotate_around(v3f{0.3f, 0.1f, 0.9f}.normalize(), Math_const::PI * 0.3f)) == v3f{-0.249471188f, 1.32052481f, 3.49198771f});
    v3fn l_rotate_normalized_result = v3f{1.0f, 2.0f, 3.0f}.normalize().rotate(quat::rotate_around(v3f{0.3f, 0.1f, 0.9f}.normalize(), Math_const::PI * 0.3f));
    assert_true(l_rotate_normalized_result == v3f{1.0f, 2.0f, 3.0f}.rotate(quat::rotate_around(v3f{0.3f, 0.1f, 0.9f}.normalize(), Math_const::PI * 0.3f)).normalize());

    // assert_true(v3f{1.0f, 2.0f, 3.0f}.rotate(quat::rotate_around(v3f{0.3f, 0.1f, 0.9f}.normalize(), Math_const::PI * 0.3f)));
    assert_true(v3f{1.0f, 2.0f, 3.0f}.euler_to_quat() == quat{-0.718287051f, 0.310622454f, 0.444435060f, 0.435952842f});

    quat l_from_to_result = quat{0.215780199f, 1.53022448e-08f, -0.0719267428f, 0.973789215f};
    assert_true(v3f{1.0f, 2.0f, 3.0f}.from_to(v3f{0.3f, 0.1f, 0.9f}) == l_from_to_result);
    assert_true(v3f{1.0f, 2.0f, 3.0f}.normalize().from_to(v3f{0.3f, 0.1f, 0.9f}) == l_from_to_result);
    assert_true(v3f{1.0f, 2.0f, 3.0f}.from_to(v3f{0.3f, 0.1f, 0.9f}.normalize()) == l_from_to_result);
    assert_true(v3f{1.0f, 2.0f, 3.0f}.normalize().from_to(v3f{0.3f, 0.1f, 0.9f}.normalize()) == l_from_to_result);

    assert_true((quat{0.2f, 0.1f, 0.8f, 0.3f} * quat{0.8f, 0.2f, 0.3f, 0.1f}) == quat{0.166666672f, 0.833333313f, 0.166666672f, -0.500000000f});
    assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.inv() == quat{-0.200000003f, -0.100000001f, -0.800000012f, 0.300000012f});
    assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.cross(quat{0.8f, 0.2f, 0.3f, 0.1f}) == quat{0.0f, 0.0f, 0.0f, 1.0f});

    assert_true(quat{0.2f, 0.1f, 0.8f, 0.3f}.to_axis() == m33f{-0.458563447f, 0.794843137f, 0.397421569f, -0.694313765f, -0.568075001f, 0.441836059f, 0.388646394f, 0.0409101509f, 0.920478284f});
    assert_true(quat{0.8f, 0.2f, 0.3f, 0.1f}.euler() == v3f{2.69183302f, -0.455598712f, 0.631079435f});

    assert_true(m33f::build_columns(v3f{1.0f, 5.0f, 0.3f}, v3f{6.0f, 0.4f, 2.3f}, v3f{9.0f, 1.0f, 0.3f}).to_rotation() == quat{0.14045234f, 0.939950287f, -0.108040266f, 0.291708738f});

    assert_true(m44f::build_translation(v3f{1.0f, 5.0f, 0.3f}) ==
                m44f::build_columns(v4f{1.0f, 0.0f, 0.0f, 0.0f}, v4f{0.0f, 1.0f, 0.0f, 0.0f}, v4f{0.0f, 0.0f, 1.0f, 0.0f}, v4f{1.0f, 5.0f, 0.3f, 1.0f}));

    assert_true(m44f::trs(v3f{1.0f, 5.0f, 0.3f}, m33f::build_columns(v3f{1.0f, 5.0f, 0.3f}.normalize().vec3, v3f{-0.2f, 3.0f, 1.3f}.normalize().vec3, v3f{2.8f, -6.4f, 4.3f}.normalize().vec3),
                          v3f{1.0f, 3.0f, 0.2f}) == m44f::build_columns(v4f{0.19577758f, 0.978887915f, 0.058733277f, 0.0f}, v4f{-0.183168754f, 2.74753118f, 1.19059682f, 0.0f},
                                                                        v4f{0.0682673007f, -0.156039551f, 0.104839079f, 0.0f}, v4f{1.0f, 5.0f, 0.300000012f, 1.0f}));

    m33f l_test = m33f::lookat(v3f{0.0f, 0.0f, 0.0f}, v3f{0.0f, 0.0f, 1.0f}, v3fn{0.0f, 1.0f, 0.0f});

    assert_true(m33f::lookat(v3f{18.0f, 1.0f, -6.0f}, v3f{-14.0f, 7.0f, -2.0f}, v3fn{0.0f, 1.0f, 0.0f}) ==
                m33f::build_columns(v3f{0.12403474f, -0.0f, 0.99227792f}, v3f{0.181500763f, 0.983129143f, -0.0226875953f}, v3f{-0.97553718f, 0.182913214f, 0.121942148f}));

    assert_true(m44f::lookat_rotation(v3f{1.0f, 5.0f, 0.3f}, v3f{-0.2f, 3.0f, 1.3f}, v3f{2.8f, -6.4f, 4.3f}) ==
                m44f::build_columns(v4f{0.140679255f, -0.509002984f, -0.849190891f, 0.000000000f}, v4f{0.869831562f, -0.346118361f, 0.351561159f, 0.000000000f},
                                    v4f{-0.472866267f, -0.788110435f, 0.394055188f, 0.000000000f}, v4f{0.000000000f, 0.000000000f, 0.000000000f, 1.00000000f}));
    assert_true(m44f::view(v3f{1.0f, 5.0f, 0.3f}, v3f{-0.2f, 3.0f, 1.3f}, v3f{2.8f, -6.4f, 4.3f}) ==
                m44f::build_columns(v4f{0.929457486f, -0.363841951f, -0.0610562637f, -0.000000000f}, v4f{0.197104529f, 0.349828482f, 0.915843844f, -0.000000000f},
                                    v4f{-0.311863184f, -0.863272250f, 0.396865606f, -0.000000000f}, v4f{-1.82142115f, -1.12631893f, -4.63722277f, 1.00000000f}));
    assert_true(m44f::perspective(1.0f, 2.0f, 3.0f, 4.0f) == m44f::build_columns(v4f{0.915243864f, 0.000000000f, 0.000000000f, 0.000000000f},
                                                                                 v4f{0.000000000f, 1.83048773f, 0.000000000f, 0.000000000f}, v4f{0.000000000f, 0.000000000f, 7.00000000f, 1.00000000f},
                                                                                 v4f{0.000000000f, 0.000000000f, -24.0000000f, 0.000000000f}));
};

inline void color_tests()
{
    color l_rgb_linear = color{100, 150, 200, 255};
    assert_true(l_rgb_linear.to_color_f() == color_f{100.0f / 255, 150.0f / 255, 200.0f / 255, 255.0f / 255});
    assert_true(l_rgb_linear.linear_to_sRGB() == l_rgb_linear.to_color_f().linear_to_sRGB().to_uint8_color());
    assert_true(l_rgb_linear.linear_to_sRGB() == color{168, 202, 229, 255});

    color l_srgb = color{168, 202, 229, 255};
    assert_true(l_srgb.sRGB_to_linear() == l_srgb.to_color_f().sRGB_to_linear().to_uint8_color());
    assert_true(l_srgb.sRGB_to_linear() == color{100, 151, 200, 255});
};

inline void assert_obb_overlap(const obb& p_left, const obb& p_right, const int8 p_overlap_result)
{
    assert_true(p_left.overlap(p_right) == p_overlap_result);
    assert_true(p_left.overlap1(p_right) == p_overlap_result);
    assert_true(p_left.overlap2(p_right) == p_overlap_result);
};

inline void geometry(){

    {aabb l_aabb_0 = aabb{v3f{0.0f, 0.5f, 0.0f}, v3f{2.0f, 2.0f, 2.0f}};
aabb l_aabb_1 = aabb{v3f{0.0f, 0.0f, 0.5f}, v3f{0.5f, 0.5f, 0.5f}};

assert_true(l_aabb_0.overlap(l_aabb_1));

l_aabb_0 = aabb{v3f{2.0f, 0.5f, 0.0f}, v3f{2.0f, 2.0f, 2.0f}};
l_aabb_1 = aabb{v3f{-0.6f, 0.0f, 0.5f}, v3f{0.5f, 0.5f, 0.5f}};

assert_true(!l_aabb_0.overlap(l_aabb_1));
}

{
    assert_obb_overlap(obb{aabb{v3f{2.37387276f, 2.09807682f, 2.43587136f}, v3f{1.0f, 1.0f, 1.0f}},
                           m33f::build_columns(v3f{-0.469846725f, 0.866025269f, -0.171010107f}, v3f{0.342020094f, 0.000000000f, -0.939692736f}, v3f{-0.813797474f, -0.500000298f, -0.296198100f})},
                       obb{aabb{v3f{2.00000000f, 3.00000000f, 0.000000000f}, v3f{1.0f, 1.0f, 1.0f}},
                           m33f::build_columns(v3f{-0.907673478f, 0.330365956f, 0.258818865f}, v3f{0.243210196f, -0.0885212421f, 0.965925872f}, v3f{0.342019975f, 0.939692676f, 0.000000000f})},
                       true);
    assert_obb_overlap(obb{aabb{v3f{2.37387276f, 2.09807682f, 2.43587136f}, v3f{1.0f, 1.0f, 1.0f}},
                           m33f::build_columns(v3f{-0.469846725f, 0.866025269f, -0.171010107f}, v3f{0.342020094f, 0.000000000f, -0.939692736f}, v3f{-0.813797474f, -0.500000298f, -0.296198100f})},
                       obb{aabb{v3f{2.00000000f, 1.00000000f, 0.000000000f}, v3f{1.0f, 1.0f, 1.0f}}, m33f::build_columns(v3f{1.0f, 0.0f, 0.0f}, v3f{0.0f, 1.0f, 0.0f}, v3f{0.0f, 0.0f, 1.0f})}, false);
}
}
;

inline void sandbox()
{
    struct func
    {
        inline static quat tt_norm(const quat& p_left, const quat& p_right)
        {
            return quat::build_v3f_f(((p_left.Vec3s.Vec * p_right.w) + (p_right.Vec3s.Vec * p_left.w)) + p_left.Vec3s.Vec.cross(p_right.Vec3s.Vec),
                                     (p_left.w * p_right.w) - p_left.Vec3s.Vec.dot(p_right.Vec3s.Vec))
                .normalize();
        };
        inline static quat tt(const quat& p_left, const quat& p_right)
        {
            return quat::build_v3f_f(((p_left.Vec3s.Vec * p_right.w) + (p_right.Vec3s.Vec * p_left.w)) + p_left.Vec3s.Vec.cross(p_right.Vec3s.Vec),
                                     (p_left.w * p_right.w) - p_left.Vec3s.Vec.dot(p_right.Vec3s.Vec));
        };
    };
};

int main()
{
    math_tests();
    color_tests();
    geometry();
    sandbox();

    memleak_ckeck();
};

#include "Common2/_external/Syscall/backends/syscall_window_backend.hpp"