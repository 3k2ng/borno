#pragma once

#include "raylib.h"
#include "raymath.h"

#include <functional>

#include "config.h"

inline float sqr(float f) {
	return f * f;
}

struct Projectile {
	float radius;
	Color color;
	std::function<Vector2(float)> interpolate;
	float delay = 0.0f;

	float et = 0.0f;

	bool Update(float delta) {
		if (delay <= 0.0f) {
			et += delta;
		}
		else {
			delay -= delta;
		}
		Vector2 position = interpolate(et);
		if (position.x < KILLING_FIELD_TOP_LEFT.x or
			position.x > KILLING_FIELD_BOTTOM_RIGHT.x or
			position.y < KILLING_FIELD_TOP_LEFT.y or
			position.y > KILLING_FIELD_BOTTOM_RIGHT.y) {
			return true;
		}

		return false;
	}

	bool Collide(Vector2 c_position, float c_radius) {
		return Vector2DistanceSqr(interpolate(et), c_position) < sqr(radius + c_radius);
	}

	void Draw(void) {
		DrawCircleV(interpolate(et), radius, color);
	}
};
