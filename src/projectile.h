#pragma once

#include "raylib.h"
#include "raymath.h"

inline float sqr(float f) {
	return f * f;
}

constexpr float BASIC_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float BASIC_PLAYER_SHOT_SPEED = 800.0f;
constexpr float BASIC_PLAYER_SHOT_DAMAGE = 10.f;

constexpr float SPLIT_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float SPLIT_PLAYER_SHOT_V_SPEED = 600.0f;
constexpr float SPLIT_PLAYER_SHOT_H_SPEED = 600.0f;
constexpr float SPLIT_PLAYER_SHOT_DAMAGE = 6.f;

constexpr float BASIC_ENEMY_SHOT_RADIUS = 8.0f;
constexpr float BASIC_ENEMY_SHOT_SPEED = 400.0f;

struct Projectile {
	Vector2 position;
	Vector2 velocity;
	enum class Type {
		BASIC_PLAYER_SHOT,
		SPLIT_PLAYER_SHOT,
		BASIC_ENEMY_SHOT
	};
	Type type;
	bool Update(float delta) {
		switch (type)
		{
		case Projectile::Type::BASIC_PLAYER_SHOT:
			break;
		case Projectile::Type::SPLIT_PLAYER_SHOT:
			if (position.x < PLAYING_FIELD_TOP_LEFT.x and velocity.x < 0.0f) velocity.x *= -1.0f;
			else if (position.x > PLAYING_FIELD_BOTTOM_RIGHT.x and velocity.x > 0.0f) velocity.x *= -1.0f;
			break;
		case Projectile::Type::BASIC_ENEMY_SHOT:
			break;
		default:
			break;
		}

		position = Vector2Add(position, Vector2Scale(velocity, delta));

		if (position.x < KILLING_FIELD_TOP_LEFT.x or
			position.x > KILLING_FIELD_BOTTOM_RIGHT.x or
			position.y < KILLING_FIELD_TOP_LEFT.y or
			position.y > KILLING_FIELD_BOTTOM_RIGHT.y) {
			return true;
		}

		return false;
	}

	bool Collide(Vector2 c_position, float c_radius) {
		switch (type)
		{
		case Projectile::Type::BASIC_PLAYER_SHOT:
			return Vector2DistanceSqr(position, c_position) < sqr(BASIC_PLAYER_SHOT_RADIUS + c_radius);
		case Projectile::Type::SPLIT_PLAYER_SHOT:
			return Vector2DistanceSqr(position, c_position) < sqr(SPLIT_PLAYER_SHOT_RADIUS + c_radius);
		case Projectile::Type::BASIC_ENEMY_SHOT:
			return Vector2DistanceSqr(position, c_position) < sqr(BASIC_ENEMY_SHOT_RADIUS + c_radius);
		default:
			break;
		}
		return false;
	}

	void Draw(void) {
		switch (type)
		{
		case Type::BASIC_PLAYER_SHOT:
			DrawCircleV(position, BASIC_PLAYER_SHOT_RADIUS, RED);
			break;
		case Type::SPLIT_PLAYER_SHOT:
			DrawCircleV(position, SPLIT_PLAYER_SHOT_RADIUS, RED);
			break;
		case Type::BASIC_ENEMY_SHOT:
			DrawCircleV(position, BASIC_ENEMY_SHOT_RADIUS, PURPLE);
			break;
		default:
			break;
		}
	}
};