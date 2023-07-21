#pragma once

#include "raylib.h"
#include "raymath.h"

#include <memory>

#include "emitter.h"

constexpr float TESTING_DUMMY_RADIUS = 20.f;
constexpr int TESTING_DUMMY_HEALTH = 10;

constexpr float POPCORN_RADIUS = 12.f;
constexpr float POPCORN_SPEED = 400.f;
constexpr int POPCORN_HEALTH = 2;


struct Destructible {
	float radius;
	Color color;
	std::function<Vector2(float)> interpolate;

	int health;
	std::shared_ptr<Emitter> contained_emitter = nullptr;

	float et = 0.0f;

	bool Update(float delta) {
		et += delta;
		Vector2 position = interpolate(et);
		if (contained_emitter != nullptr) { contained_emitter->position = position; }

		if (position.x < KILLING_FIELD_TOP_LEFT.x or
			position.x > KILLING_FIELD_BOTTOM_RIGHT.x or
			position.y < KILLING_FIELD_TOP_LEFT.y or
			position.y > KILLING_FIELD_BOTTOM_RIGHT.y) {
			return true;
		}

		return false;
	}

	void Draw(void) {
		DrawCircleV(interpolate(et), radius, color);
	}

	inline bool Hurt(void) {
		return --health <= 0;
	}

	inline Vector2 GetPosition() {
		return interpolate(et);
	}
};

struct DestructibleSpawner {
	float cd_timer;
	Destructible destructible_to_spawn;
	bool Update(float delta) {
		if (cd_timer <= 0.0f) {
			return true;
		}
		if (cd_timer > 0.0f) {
			cd_timer -= delta;
		}
		return false;
	}
};