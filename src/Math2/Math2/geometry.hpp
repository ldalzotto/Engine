#pragma once

struct obb;
struct aabb;

struct aabb
{
    v3f center;
    v3f radiuses;

    inline int8 overlap(const aabb& p_other) const;
    inline aabb add_position(const v3f& p_position) const;
    inline obb add_position_rotation(const transform_pa& p_position_rotation) const;
};

struct obb
{
    aabb box;
    m33f axis;

    inline void extract_vertices(Slice<v3f>* in_out_vertices) const;

    inline int8 overlap(const obb& p_other) const;
    inline int8 overlap1(const obb& p_other) const;
    inline int8 overlap2(const obb& p_other) const;
};

struct SAT
{
    inline static int8 mainaxis_x(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);
    inline static int8 mainaxis_y(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);
    inline static int8 mainaxis_z(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii);

    /*
        Collapse left and right vertices to the tested axis.
        Returns 1 if the shapes doesn't intersect along the tested axis.
    */
    inline static int8 L_R(const Slice<v3f>& p_left_vertices, const Slice<v3f>& p_right_vertices, const v3f& p_tested_axis);

    /*
        This SAT test is true only if left and right centers are at the exact center of the tested shapes.
        When that's the case, the SAT::L_R can be simplified by comparing the radii of the shape. Thus, SAT calculations with vertices now become SAT calculations with axis.
        /!\ If we are coputing the SAT test for multiple axis of the same shape, it is more efficient to use the L_origin_oriented_R_oriented_symetricalshapes that skip one center projection.
    */
    inline static int8 L_oriented_R_oriented_symetricalshapes(const v3f& p_left_center, const m33f& p_left_radii_oriented, const v3f& p_right_center, const m33f& p_right_radii_oriented,
                                                              const v3f& p_tested_axis);

    /*
        This SAT test is true only if left and right centers are at the exact center of the tested shapes.
        The right center is relative to the left. So the left center is considered the origin.
        When that's the case, the SAT::L_R can be simplified by comparing the radii of the shape. Thus, SAT calculations with vertices now become SAT calculations with axis.
    */
    inline static int8 L_origin_oriented_R_oriented_symetricalshapes(const m33f& p_left_radii_oriented, const v3f& p_right_center_relative_to_left, const m33f& p_right_radii_oriented,
                                                                     const v3f& p_tested_axis);
};

struct GeometryUtil
{
    /*
        Project all points to the input axis and return the min and max projected value.
    */
    inline static void collapse_points_to_axis_min_max(const v3f& p_axis, const Slice<v3f>& p_points, float32* out_min, float32* out_max);
};

inline int8 aabb::overlap(const aabb& p_other) const
{
    if (SAT::mainaxis_x(this->center, this->radiuses, p_other.center, p_other.radiuses) | SAT::mainaxis_y(this->center, this->radiuses, p_other.center, p_other.radiuses) |
        SAT::mainaxis_z(this->center, this->radiuses, p_other.center, p_other.radiuses))
    {
        return 0;
    };

    return 1;
};

inline aabb aabb::add_position(const v3f& p_position) const
{
    return aabb{this->center + p_position, this->radiuses};
};

inline obb aabb::add_position_rotation(const transform_pa& p_position_rotation) const
{
    return obb{this->add_position(p_position_rotation.position), p_position_rotation.axis};
};

inline void obb::extract_vertices(Slice<v3f>* in_out_vertices) const
{
    v3f l_rad_delta[3];
    l_rad_delta[0] = this->axis.Points2D[0] * this->box.radiuses.Points[0];
    l_rad_delta[1] = this->axis.Points2D[1] * this->box.radiuses.Points[1];
    l_rad_delta[2] = this->axis.Points2D[2] * this->box.radiuses.Points[2];

    *Slice_get(in_out_vertices, 0) = this->box.center + l_rad_delta[0] + l_rad_delta[1] + l_rad_delta[2];
    *Slice_get(in_out_vertices, 1) = this->box.center + l_rad_delta[0] - l_rad_delta[1] + l_rad_delta[2];
    *Slice_get(in_out_vertices, 2) = this->box.center + l_rad_delta[0] + l_rad_delta[1] - l_rad_delta[2];
    *Slice_get(in_out_vertices, 3) = this->box.center + l_rad_delta[0] - l_rad_delta[1] - l_rad_delta[2];

    *Slice_get(in_out_vertices, 4) = this->box.center - l_rad_delta[0] + l_rad_delta[1] + l_rad_delta[2];
    *Slice_get(in_out_vertices, 5) = this->box.center - l_rad_delta[0] - l_rad_delta[1] + l_rad_delta[2];
    *Slice_get(in_out_vertices, 6) = this->box.center - l_rad_delta[0] + l_rad_delta[1] - l_rad_delta[2];
    *Slice_get(in_out_vertices, 7) = this->box.center - l_rad_delta[0] - l_rad_delta[1] - l_rad_delta[2];
};

inline int8 obb::overlap(const obb& p_other) const
{
    v3f p_left_points_arr[8];
    v3f p_right_points_arr[8];
    Slice<v3f> p_left_points = Slice_build_memory_elementnb<v3f>(p_left_points_arr, 8);
    Slice<v3f> p_right_points = Slice_build_memory_elementnb<v3f>(p_right_points_arr, 8);

    this->extract_vertices(&p_left_points);
    p_other.extract_vertices(&p_right_points);

    // If at least one test is successful (meaning that shapes are not overlapping along one SAT) then thes shapes are not overlapping
    if (SAT::L_R(p_left_points, p_right_points, this->axis.Right) | SAT::L_R(p_left_points, p_right_points, this->axis.Up) | SAT::L_R(p_left_points, p_right_points, this->axis.Forward) |
        SAT::L_R(p_left_points, p_right_points, p_other.axis.Right) | SAT::L_R(p_left_points, p_right_points, p_other.axis.Up) | SAT::L_R(p_left_points, p_right_points, p_other.axis.Forward) |
        SAT::L_R(p_left_points, p_right_points, this->axis.Right.cross(p_other.axis.Right)) | SAT::L_R(p_left_points, p_right_points, this->axis.Right.cross(p_other.axis.Up)) |
        SAT::L_R(p_left_points, p_right_points, this->axis.Right.cross(p_other.axis.Forward)) | SAT::L_R(p_left_points, p_right_points, this->axis.Up.cross(p_other.axis.Right)) |
        SAT::L_R(p_left_points, p_right_points, this->axis.Up.cross(p_other.axis.Up)) | SAT::L_R(p_left_points, p_right_points, this->axis.Up.cross(p_other.axis.Forward)) |
        SAT::L_R(p_left_points, p_right_points, this->axis.Forward.cross(p_other.axis.Right)) | SAT::L_R(p_left_points, p_right_points, this->axis.Forward.cross(p_other.axis.Up)) |
        SAT::L_R(p_left_points, p_right_points, this->axis.Forward.cross(p_other.axis.Forward)))
    {
        return 0;
    }

    return 1;
};

inline int8 obb::overlap1(const obb& p_other) const
{
    m33f l_this_box_radii_oriented;
    m33f l_other_radii_orented;
    for (int8 i = 0; i < 3; i++)
    {
        l_this_box_radii_oriented.Points2D[i] = (this->axis.Points2D[i] * this->box.radiuses);
        l_other_radii_orented.Points2D[i] = (p_other.axis.Points2D[i] * p_other.box.radiuses);
    }

    // If at least one test is successful (meaning that shapes are not overlapping along one SAT) then thes shapes are not overlapping
    if (SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Right) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Up) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Forward) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, p_other.axis.Right) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, p_other.axis.Up) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, p_other.axis.Forward)

        | SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Right)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Up)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Forward)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Right)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Up)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Forward)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Right)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Up)) |
        SAT::L_oriented_R_oriented_symetricalshapes(this->box.center, l_this_box_radii_oriented, p_other.box.center, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Forward)))
    {
        return 0;
    }

    return 1;
};

inline int8 obb::overlap2(const obb& p_other) const
{
    m33f l_this_box_radii_oriented;
    m33f l_other_radii_orented;
    for (int8 i = 0; i < 3; i++)
    {
        l_this_box_radii_oriented.Points2D[i] = (this->axis.Points2D[i] * this->box.radiuses);
        l_other_radii_orented.Points2D[i] = (p_other.axis.Points2D[i] * p_other.box.radiuses);
    }

    v3f l_other_center_relative_to_this = p_other.box.center - this->box.center;

    // If at least one test is successful (meaning that shapes are not overlapping along one SAT) then thes shapes are not overlapping
    if (SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Right) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Up) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Forward) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, p_other.axis.Right) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, p_other.axis.Up) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, p_other.axis.Forward)

        | SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Right)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Up)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Right.cross(p_other.axis.Forward)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Right)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Up)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Up.cross(p_other.axis.Forward)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Right)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Up)) |
        SAT::L_origin_oriented_R_oriented_symetricalshapes(l_this_box_radii_oriented, l_other_center_relative_to_this, l_other_radii_orented, this->axis.Forward.cross(p_other.axis.Forward)))
    {
        return 0;
    }

    return 1;
};

// see Christer_Ericson-Real-Time_Collision_Detection-EN
#if 0
inline int8 obb::overlap2(const obb& p_other) const
{
    float32 l_left_radii_projected, l_right_radii_projected;
    m33f l_radii, l_radii_abs;
    v3f l_t;

    for (uint8 i = 0; i < 3; i++)
    {
        for (uint8 j = 0; j < 3; j++)
        {
            l_radii.Points2D[i].Points[j] = this->axis.Points2D[i].dot(p_other.axis.Points2D[j]);
            l_radii_abs.Points2D[i].Points[j] = fabsf(l_radii.Points2D[i].Points[j]) + (float32)Limits::tol_f;
        }
    }

    l_t = p_other.box.center - this->box.center;
    l_t = v3f{l_t.dot(this->axis.Points2D[0]), l_t.dot(this->axis.Points2D[1]), l_t.dot(this->axis.Points2D[2])};

    for (uint8 i = 0; i < 3; i++)
    {
        l_left_radii_projected = this->box.radiuses.Points[i];
        l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[i].Points[0]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[i].Points[1]) +
                                  (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[i].Points[2]);
        if (fabsf(l_t.Points[i]) > (l_left_radii_projected + l_right_radii_projected))
            return false;
    }

    for (uint8 i = 0; i < 3; i++)
    {
        l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[i]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[i]) +
                                 (this->box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[i]);
        l_right_radii_projected = p_other.box.radiuses.Points[i];
        if (fabsf((l_t.Points[0] * l_radii.Points2D[0].Points[i]) + (l_t.Points[1] * l_radii.Points2D[1].Points[i]) + (l_t.Points[2] * l_radii.Points2D[2].Points[i])) >
            (l_left_radii_projected + l_right_radii_projected))
            return false;
    }

    l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
    l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
    if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[0]) - (l_t.Points[1] * l_radii.Points2D[2].Points[0])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[1]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
    if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[1]) - (l_t.Points[1] * l_radii.Points2D[2].Points[1])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[2]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[0].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
    if (fabsf((l_t.Points[2] * l_radii.Points2D[1].Points[2]) - (l_t.Points[1] * l_radii.Points2D[2].Points[2])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[0]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[0]);
    l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[1]);
    if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[0]) - (l_t.Points[2] * l_radii.Points2D[0].Points[0])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[1]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[1].Points[0]);
    if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[1]) - (l_t.Points[2] * l_radii.Points2D[0].Points[1])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (this->box.radiuses.Points[2] * l_radii_abs.Points2D[0].Points[2]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[1].Points[0]);
    if (fabsf((l_t.Points[0] * l_radii.Points2D[2].Points[2]) - (l_t.Points[2] * l_radii.Points2D[0].Points[2])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[0]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[0]);
    l_right_radii_projected = (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[1]);
    if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[0]) - (l_t.Points[0] * l_radii.Points2D[1].Points[0])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[1]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[1]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[2]) + (p_other.box.radiuses.Points[2] * l_radii_abs.Points2D[2].Points[0]);
    if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[1]) - (l_t.Points[0] * l_radii.Points2D[1].Points[1])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    l_left_radii_projected = (this->box.radiuses.Points[0] * l_radii_abs.Points2D[1].Points[2]) + (this->box.radiuses.Points[1] * l_radii_abs.Points2D[0].Points[2]);
    l_right_radii_projected = (p_other.box.radiuses.Points[0] * l_radii_abs.Points2D[2].Points[1]) + (p_other.box.radiuses.Points[1] * l_radii_abs.Points2D[2].Points[0]);
    if (fabsf((l_t.Points[1] * l_radii.Points2D[0].Points[2]) - (l_t.Points[0] * l_radii.Points2D[1].Points[2])) > (l_left_radii_projected + l_right_radii_projected))
        return false;

    return true;
};
#endif

inline int8 SAT::mainaxis_x(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
    return (Math::greater(fabsf(p_left_center.x - p_right_center.x), p_left_radii.x + p_right_radii.x));
};

inline int8 SAT::mainaxis_y(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
    return (Math::greater(fabsf(p_left_center.y - p_right_center.y), p_left_radii.y + p_right_radii.y));
};

inline int8 SAT::mainaxis_z(const v3f& p_left_center, const v3f& p_left_radii, const v3f& p_right_center, const v3f& p_right_radii)
{
    return (Math::greater(fabsf(p_left_center.z - p_right_center.z), p_left_radii.z + p_right_radii.z));
};

inline int8 SAT::L_R(const Slice<v3f>& p_left_vertices, const Slice<v3f>& p_right_vertices, const v3f& p_tested_axis)
{
    float32 l_left_min, l_left_max, l_right_min, l_right_max;
    GeometryUtil::collapse_points_to_axis_min_max(p_tested_axis, p_left_vertices, &l_left_min, &l_left_max);
    GeometryUtil::collapse_points_to_axis_min_max(p_tested_axis, p_right_vertices, &l_right_min, &l_right_max);

    return (l_left_min < l_right_min || l_left_min > l_right_max) && (l_right_min < l_left_min || l_right_min > l_left_max);
};

inline int8 SAT::L_oriented_R_oriented_symetricalshapes(const v3f& p_left_center, const m33f& p_left_radii_oriented, const v3f& p_right_center, const m33f& p_right_radii_oriented,
                                                        const v3f& p_tested_axis)
{
    float32 l_left_radii_projected = 0;
    float32 l_right_radii_projected = 0;
    for (int8 j = 0; j < 3; j++)
    {
        l_left_radii_projected += fabsf(p_left_radii_oriented.Points2D[j].dot(p_tested_axis));
        l_right_radii_projected += fabsf(p_right_radii_oriented.Points2D[j].dot(p_tested_axis));
    }

    float32 l_left_center_projected = p_left_center.dot(p_tested_axis);
    float32 l_right_center_projected = p_right_center.dot(p_tested_axis);

    float32 l_tl = fabsf(l_right_center_projected - l_left_center_projected);

    return Math::greater_eq(l_tl, l_left_radii_projected + l_right_radii_projected);
};

inline int8 SAT::L_origin_oriented_R_oriented_symetricalshapes(const m33f& p_left_radii_oriented, const v3f& p_right_center_relative_to_left, const m33f& p_right_radii_oriented,
                                                               const v3f& p_tested_axis)
{
    float32 l_left_radii_projected = 0;
    float32 l_right_radii_projected = 0;
    for (int8 j = 0; j < 3; j++)
    {
        l_left_radii_projected += fabsf(p_left_radii_oriented.Points2D[j].dot(p_tested_axis));
        l_right_radii_projected += fabsf(p_right_radii_oriented.Points2D[j].dot(p_tested_axis));
    }

    float32 l_right_center_projected = p_right_center_relative_to_left.dot(p_tested_axis);

    float32 l_tl = fabsf(l_right_center_projected);

    return Math::greater_eq(l_tl, l_left_radii_projected + l_right_radii_projected);
};

inline void GeometryUtil::collapse_points_to_axis_min_max(const v3f& p_axis, const Slice<v3f>& p_points, float32* out_min, float32* out_max)
{
    *out_min = FLT_MAX;
    *out_max = -FLT_MAX;

    for (uimax i = 0; i < p_points.Size; i++)
    {
        float32 l_proj = Slice_get(&p_points, i)->dot(p_axis);
        if (Math::lower(l_proj, *out_min))
        {
            *out_min = l_proj;
        }
        if (Math::greater(l_proj, *out_max))
        {
            *out_max = l_proj;
        }
    }
};