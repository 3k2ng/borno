#pragma once

#include "raylib.h"
#include "raymath.h"

#include <functional>
#include <vector>

constexpr float BASIC_ENEMY_SHOT_RADIUS = 8.0f;
constexpr float BASIC_ENEMY_SHOT_SPEED = 400.0f;

std::function<std::vector<ProjectileSpawner>(float, float, Vector2, Vector2)> single_aimed_shot(float cd) {
	return [=](float et, float dt, Vector2 ep, Vector2 pp) -> std::vector<ProjectileSpawner> {
		if (floorf((et + dt) / cd) > floorf(et / cd)) return {
			ProjectileSpawner{
				floorf((et + dt) / cd) * cd,
				Projectile{ BASIC_ENEMY_SHOT_RADIUS, PURPLE, linear(ep, Vector2Scale(Vector2Normalize(Vector2Subtract(pp, ep)), BASIC_ENEMY_SHOT_SPEED)), 0.1f}
			}
		};
		return {};
	};
}

std::function<std::vector<ProjectileSpawner>(float, float, Vector2, Vector2)> linear_ring(int shots, float cd, float initial_angle) {
	float segment_angle = 2.0f * PI / float(shots);
	return [=](float et, float dt, Vector2 ep, Vector2 pp) -> std::vector<ProjectileSpawner> {
		if (floorf((et + dt) / cd) > floorf(et / cd)) {
			float tts = floorf((et + dt) / cd) * cd;
			std::vector<ProjectileSpawner> pss;
			for (int i = 0; i < shots; i++) {
				float current_angle = initial_angle + float(i) * segment_angle;
				pss.push_back(ProjectileSpawner{
					tts,
					Projectile{ BASIC_ENEMY_SHOT_RADIUS, PURPLE, linear(ep, Vector2{ cosf(current_angle) * BASIC_ENEMY_SHOT_SPEED, sinf(current_angle) * BASIC_ENEMY_SHOT_SPEED }), 0.1f}
					});
			}

			return pss;
		}
		return {};
	};
}

std::function<std::vector<ProjectileSpawner>(float, float, Vector2, Vector2)> linear_spinny_ring(int shots, float cd, float initial_angle, float duration, float shot_interval, float spinny_angle) {
	float segment_angle = 2.0f * PI / float(shots);
	return [=](float et, float dt, Vector2 ep, Vector2 pp) -> std::vector<ProjectileSpawner> {
		if (floorf((et + dt) / cd) > floorf(et / cd)) {
			float tts = floorf((et + dt) / cd) * cd;
			std::vector<ProjectileSpawner> pss;
			for (float t = 0.0f; t <= duration; t += shot_interval) {
				float offset_angle = spinny_angle * t / duration;
				for (int i = 0; i < shots; i++) {
					float current_angle = initial_angle + offset_angle + float(i) * segment_angle;
					pss.push_back(ProjectileSpawner{
						tts + t,
						Projectile{ BASIC_ENEMY_SHOT_RADIUS, PURPLE, linear(ep, Vector2{ cosf(current_angle) * BASIC_ENEMY_SHOT_SPEED, sinf(current_angle) * BASIC_ENEMY_SHOT_SPEED }), 0.1f}
						});
				}
			}

			return pss;
		}
		return {};
	};
}

std::function<std::vector<ProjectileSpawner>(float, float, Vector2, Vector2)> linear_aim_ring_pattern(int shots, float radius, float cd, float initial_angle) {
	float segment_angle = 2.0f * PI / float(shots);
	return [=](float et, float dt, Vector2 ep, Vector2 pp) -> std::vector<ProjectileSpawner> {
		if (floorf((et + dt) / cd) > floorf(et / cd)) {
			float tts = floorf((et + dt) / cd) * cd;
			std::vector<ProjectileSpawner> pss;
			for (int i = 0; i < shots; i++) {
				float current_angle = initial_angle + float(i) * segment_angle;
				pss.push_back(ProjectileSpawner{
					tts,
					Projectile{ BASIC_ENEMY_SHOT_RADIUS, PURPLE, linear(Vector2{ ep.x + cosf(current_angle) * radius, ep.y + sinf(current_angle) * radius }, Vector2Scale(Vector2Normalize(Vector2Subtract(pp, ep)), BASIC_ENEMY_SHOT_SPEED)), 0.1f}
					});
			}

			return pss;
		}
		return {};
	};
}