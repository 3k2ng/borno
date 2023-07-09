#pragma once

#include "raylib.h"
#include "raymath.h"

#include "emitter.h"

constexpr float TESTING_DUMMY_RADIUS = 20.f;
constexpr float TESTING_DUMMY_HEALTH = 100.f;

constexpr float POPCORN_0_RADIUS = 12.f;
constexpr float POPCORN_0_SPEED = 400.f;
constexpr float POPCORN_0_HEALTH = 12.0f;


struct Destructible {
	Vector2 position;
	Vector2 velocity;
	float health;
	Emitter* contained_emitter;
	enum class Type {
		TESTING_DUMMY,
		POPCORN_0
	};
	Type type;

	Destructible(Vector2 initial_pos, Destructible::Type t, Emitter* e, Vector2 v = Vector2Zero()) {
		position = initial_pos;
		type = t;
		contained_emitter = e;
		velocity = v;
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			health = TESTING_DUMMY_HEALTH;
			break;
		case Destructible::Type::POPCORN_0:
			health = POPCORN_0_HEALTH;
			break;
		default:
			break;
		}
	}

	bool Update(float delta) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			break;
		case Destructible::Type::POPCORN_0:
			break;
		default:
			break;
		}

		position = Vector2Add(position, Vector2Scale(velocity, delta));
		if (contained_emitter != nullptr) contained_emitter->position = position;

		if (position.x < KILLING_FIELD_TOP_LEFT.x or
			position.x > KILLING_FIELD_BOTTOM_RIGHT.x or
			position.y < KILLING_FIELD_TOP_LEFT.y or
			position.y > KILLING_FIELD_BOTTOM_RIGHT.y) {
			return true;
		}

		return false;
	}

	void Draw(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			DrawCircleV(position, TESTING_DUMMY_RADIUS, BLUE);
			break;
		case Destructible::Type::POPCORN_0:
			DrawCircleV(position, POPCORN_0_RADIUS, BLUE);
			break;
		default:
			break;
		}
	}

	bool Hurt(float damage_value) {
		health -= damage_value;
		if (health <= 0.0f) {
			return true;
		}
		return false;
	}

	float GetRadius(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			return TESTING_DUMMY_RADIUS;
		case Destructible::Type::POPCORN_0:
			return POPCORN_0_RADIUS;
		default:
			return 0.0f;
		}
	}
};