#pragma once

#include "raylib.h"
#include "raymath.h"

#include <queue>
#include <memory>

#include "projectile.h"

constexpr float TESTING_EMITTER_SHOT_CD = 1.0f;

struct ProjectileSpawner {
	float time_to_spawn;
	Projectile projectile_to_spawn;
};

struct PSCompare {
	bool operator() (ProjectileSpawner& ls, ProjectileSpawner& rs) {
		return ls.time_to_spawn > rs.time_to_spawn;
	};
};

struct Emitter {
	std::function<std::vector<ProjectileSpawner>(float, float, Vector2, Vector2)> spawn_fn;
	std::list<std::shared_ptr<Emitter>>::iterator it;
	Vector2 position;
	std::priority_queue<ProjectileSpawner, std::vector<ProjectileSpawner>, PSCompare> ps_queue;
	float et = 0.0f;
	std::vector<Projectile> Update(float delta, Vector2 player_pos) {
		std::vector<ProjectileSpawner> nps = spawn_fn(et, delta, position, player_pos);
		for (ProjectileSpawner ps : nps) {
			ps_queue.push(ps);
		}
		std::vector<Projectile> pts;
		et += delta;
		while (not ps_queue.empty() and ps_queue.top().time_to_spawn <= et) {
			pts.push_back(ps_queue.top().projectile_to_spawn);
			ps_queue.pop();
		}
		return pts;
	}
};