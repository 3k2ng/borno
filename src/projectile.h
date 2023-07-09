#pragma once

#include "raylib.h"
#include "raymath.h"

inline float sqr(float f) {
	return f * f;
}

constexpr float BASIC_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float BASIC_PLAYER_SHOT_SPEED = 800.0f;

constexpr float SPLIT_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float SPLIT_PLAYER_SHOT_H_SPEED = 600.0f;
constexpr float SPLIT_PLAYER_SHOT_V_SPEED = 600.0f;

constexpr float BASIC_ENEMY_SHOT_RADIUS = 8.0f;
constexpr float BASIC_ENEMY_SHOT_SPEED = 400.0f;

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
