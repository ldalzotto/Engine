#include "Collision/collision.hpp"


/*
	3 BoxColliders that intersect each other : (1,2,3)
	ColliderDetector attached to 1
		-> B1 generates two TRIGGER_ENTER ITriggerEvent with B2 and B3
	Nothing happens
		-> B1 have two TRIGGER_STAY ITriggerEvent with B2 and B3
	BoxCollider1 moves away
		-> B1 have two TRIGGER_EXIT ITriggerEvent with B2 and B3
	Nothing happens
		-> B1 have two NONE ITriggerEvent with B2 and B3
*/
inline void collision_test_01()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{0.0f, 0.0f, 0.0f}, v3f{0.5f, 0.5f, 0.5f} };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_box_collider_1_transform = transform_pa{ v3f{1000000.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
	l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.free_collider(l_box_collider_1);
	l_collision.free_collider(l_box_collider_2);
	l_collision.free_collider(l_box_collider_3);

	l_collision.free();

}

/*
	3 BoxColliders that intersect each other : (1,2,3)
	ColliderDetector attached to 1
	B1 ITriggerEvent is already TRIGGER_STAY with B2 and B3
	BoxCollider2 moves away
		-> B1~B2 : TRIGGER_EXIT, B1~B3 : TRIGGER_STAY
	Nothing happens
		-> B1~B2 : NONE, B1~B3 : TRIGGER_STAY
	Deleting the B3
		-> B1~B2 : NONE, B1~B3 : TRIGGER_EXIT
	Nothing happens
		-> B1~B2 : NONE
*/
inline void collision_test_02()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{ 0.0f, 0.0f, 0.0f }, v3f{ 0.5f, 0.5f, 0.5f } };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle;
	{
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
	}

	l_collision.step();

	l_collision.step();

	l_box_collider_2_transform = transform_pa{ v3f{1000000.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
	l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.free_collider(l_box_collider_3);

	//Taking deletion into account
	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);
		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_collision.free_collider(l_box_collider_1);
	l_collision.free_collider(l_box_collider_2);

	l_collision.free();
}



/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to all.
		-> B1~B2 : TRIGGER_ENTER, B1~B3 : TRIGGER_ENTER
		   B2~B1 : TRIGGER_ENTER, B2~B3 : TRIGGER_ENTER
		   B3~B1 : TRIGGER_ENTER, B3~B2 : TRIGGER_ENTER
	BoxCollider2 moves away
		-> B1~B2 : TRIGGER_EXIT, B1~B3 : TRIGGER_STAY
		   B2~B1 : TRIGGER_EXIT, B2~B3 : TRIGGER_EXIT
		   B3~B1 : TRIGGER_STAY, B3~B2 : TRIGGER_EXIT
	BoxCollider1 destroyed
		-> B1~B2 : NONE, B1~B3 : TRIGGER_EXIT
		   B2~B1 : TRIGGER_EXIT, B2~B3 : NONE
		   B3~B1 : TRIGGER_EXIT, B3~B2 : NONE
	Nothing happens
		-> B2~B3 : NONE
		   B3~B2 : NONE
*/
inline void collision_test_03()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{0.0f, 0.0f, 0.0f}, v3f{0.5f, 0.5f, 0.5f} };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_box_collider_2_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_2);
		l_box_collider_3_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_3);
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_2_events = l_collision.get_collision_events(l_box_collider_2_detector_handle);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_2_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_2_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_ENTER);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}

	l_box_collider_2_transform = transform_pa{ v3f{1000000.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
	l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_2_events = l_collision.get_collision_events(l_box_collider_2_detector_handle);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_2_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_2_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}

	l_collision.free_collider(l_box_collider_1);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_2_events = l_collision.get_collision_events(l_box_collider_2_detector_handle);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_2_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_2_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_2_events = l_collision.get_collision_events(l_box_collider_2_detector_handle);

		assert_true(l_box_collider_2_events.Size == 2);
		assert_true(l_box_collider_2_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_2_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_2_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_2_events.get(1).other, l_box_collider_3));
	}
	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}

	l_collision.free_collider(l_box_collider_2);
	l_collision.free_collider(l_box_collider_3);

	l_collision.free();
}



/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to all.
	All trigger are already TRIGGER_STAY
	BoxCollider1 destroyed
	ColliderDetector2 destroyed
		-> B3-B1 : TRIGGER_EXIT, B3~B2 : TRIGGER_STAY
	Creating a new BoxCollider far away
		-> B3-B1 : NONE, B3~B2 : TRIGGER_STAY
*/
inline void collision_test_04()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{0.0f, 0.0f, 0.0f}, v3f{0.5f, 0.5f, 0.5f} };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_box_collider_2_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_2);
		l_box_collider_3_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_3);
	}

	l_collision.step();

	l_collision.free_collider(l_box_collider_1);
	l_collision.free_colliderdetector(l_box_collider_2, l_box_collider_2_detector_handle);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}


	transform_pa l_box_collider_4_transform;
	{
		l_box_collider_4_transform = transform_pa{ v3f{100000.0f, 100000.0f, 100000.0f}, quat_const::IDENTITY.to_axis() };
	}
	Token(BoxCollider) l_box_collider_4;
	l_box_collider_4 = l_collision.allocate_boxcollider(l_unit_aabb);

	l_collision.step();


	{
		Slice<TriggerEvent> l_box_collider_3_events = l_collision.get_collision_events(l_box_collider_3_detector_handle);

		assert_true(l_box_collider_3_events.Size == 2);
		assert_true(l_box_collider_3_events.get(0).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_3_events.get(0).other, l_box_collider_1));
		assert_true(l_box_collider_3_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_3_events.get(1).other, l_box_collider_2));
	}


	l_collision.free_collider(l_box_collider_2);
	l_collision.free_collider(l_box_collider_3);
	l_collision.free_collider(l_box_collider_4);

	l_collision.free();
}

/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to all.
	All trigger are already TRIGGER_STAY.
	On the same frame:
	destroy everything, create and destroy again.
	create a BoxCollider with ColliderDetector at the same position of B1
		-> B1 has no trigger events
*/
inline void collision_test_05()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{0.0f, 0.0f, 0.0f}, v3f{0.5f, 0.5f, 0.5f} };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle, l_box_collider_2_detector_handle, l_box_collider_3_detector_handle;
	{
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_box_collider_2_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_2);
		l_box_collider_3_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_3);
	}

	l_collision.step();

	{
		l_collision.free_collider(l_box_collider_1);
		l_collision.free_collider(l_box_collider_2);
		l_collision.free_collider(l_box_collider_3);

		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_box_collider_2_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_2);
		l_box_collider_3_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_3);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);

		l_collision.free_collider(l_box_collider_1);
		l_collision.free_collider(l_box_collider_2);
		l_collision.free_collider(l_box_collider_3);

		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
	}

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);
		assert_true(l_box_collider_1_events.Size == 0);
	}


	l_collision.free_collider(l_box_collider_1);

	l_collision.free();
};

/*
	3 BoxColliders that intersect each other : (1,2,3).
	ColliderDetector attached to 1.
	All trigger are already TRIGGER_STAY.
	Move B3 away
		-> B1~B3 : TRIGGER_EXIT
	Move B3 away again
		-> B1~B3 : NONE
*/
inline void collision_test_06()
{
	Collision2 l_collision = Collision2::allocate();

	aabb l_unit_aabb = aabb{ v3f{0.0f, 0.0f, 0.0f}, v3f{0.5f, 0.5f, 0.5f} };
	transform_pa l_box_collider_1_transform, l_box_collider_2_transform, l_box_collider_3_transform;
	{
		l_box_collider_1_transform = transform_pa{ v3f{0.0f, 0.0f, 0.0f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_2_transform = transform_pa{ v3f{0.25f, 0.0f, 0.25f}, quat_const::IDENTITY.to_axis() };
		l_box_collider_3_transform = transform_pa{ v3f{0.25f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	}

	Token(BoxCollider) l_box_collider_1, l_box_collider_2, l_box_collider_3;
	{
		l_box_collider_1 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_2 = l_collision.allocate_boxcollider(l_unit_aabb);
		l_box_collider_3 = l_collision.allocate_boxcollider(l_unit_aabb);

		l_collision.on_collider_moved(l_box_collider_1, l_box_collider_1_transform);
		l_collision.on_collider_moved(l_box_collider_2, l_box_collider_2_transform);
		l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);
	}

	Token(ColliderDetector) l_box_collider_1_detector_handle, l_box_collider_2_detector_handle;
	{
		l_box_collider_1_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_1);
		l_box_collider_2_detector_handle = l_collision.allocate_colliderdetector(l_box_collider_2);
	}

	l_collision.step();
	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}

	l_box_collider_3_transform = transform_pa{ v3f{20.0f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::TRIGGER_EXIT);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}


	l_box_collider_3_transform = transform_pa{ v3f{22.0f, 0.0f, -0.25f}, quat_const::IDENTITY.to_axis() };
	l_collision.on_collider_moved(l_box_collider_3, l_box_collider_3_transform);

	l_collision.step();

	{
		Slice<TriggerEvent> l_box_collider_1_events = l_collision.get_collision_events(l_box_collider_1_detector_handle);

		assert_true(l_box_collider_1_events.Size == 2);
		assert_true(l_box_collider_1_events.get(0).state == Trigger::State::TRIGGER_STAY);
		assert_true(tk_eq(l_box_collider_1_events.get(0).other, l_box_collider_2));
		assert_true(l_box_collider_1_events.get(1).state == Trigger::State::NONE);
		assert_true(tk_eq(l_box_collider_1_events.get(1).other, l_box_collider_3));
	}


	l_collision.free_collider(l_box_collider_1);
	l_collision.free_collider(l_box_collider_2);
	l_collision.free_collider(l_box_collider_3);

	l_collision.free();
};

int main()
{
	collision_test_01();
	collision_test_02();
	collision_test_03();
	collision_test_04();
	collision_test_05();
	collision_test_06();
};