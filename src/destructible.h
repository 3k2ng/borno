#pragma once

#include "raylib.h"
#include "raymath.h"

#include "emitter.h"

constexpr float TESTING_DUMMY_RADIUS = 20.f;

struct Destructible {
	Vector2 position;
	Vector2 velocity;
	std::list<Emitter>::iterator ce_it;
	enum class Type {
		TESTING_DUMMY
	};
	Type type;

	Destructible(Vector2 p, Destructible::Type d_type, std::list<Emitter>::iterator it) {
		position = p;
		type = d_type;
		ce_it = it;
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			velocity = Vector2Zero();
			break;
		default:
			break;
		}
	}

	void Draw(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			DrawCircleV(position, TESTING_DUMMY_RADIUS, BLUE);
			break;
		default:
			break;
		}
	}

	bool Hurt(float damage_value) {
		return false;
	}

	float GetRadius(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			return TESTING_DUMMY_RADIUS;
		default:
			return 0.0f;
		}
	}
};