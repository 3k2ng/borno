#pragma once

#include "raylib.h"
#include "raymath.h"

#include "projectile.h"

constexpr float TESTING_EMITTER_SHOT_CD = 1.0f;

struct Emitter {
	std::list<Emitter>::iterator it;
	Vector2 position;
	float shoot_timer;
	enum class Type {
		TESTING_EMITTER
	};
	Type type;
	std::vector<Projectile> Update(float delta, Vector2 player_pos) {
		std::vector<Projectile> ep_to_insert;
		if (shoot_timer <= 0.0f) {
			switch (type)
			{
			case Emitter::Type::TESTING_EMITTER:
				ep_to_insert.push_back(
					Projectile{
						position,
						Vector2Scale(Vector2Normalize(Vector2Subtract(player_pos, position)), BASIC_ENEMY_SHOT_SPEED),
						Projectile::Type::BASIC_ENEMY_SHOT
					});
				shoot_timer = TESTING_EMITTER_SHOT_CD;
				break;
			default:
				break;
			}
		}
		else {
			shoot_timer -= delta;
		}
		return ep_to_insert;
	}
};