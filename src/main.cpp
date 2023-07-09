#include "raylib.h"
#include "raymath.h"

#include <list>
#include <vector>
#include <optional>
#include <queue>
#include <functional>

#include <iostream>

#include "config.h"

#include "emitter.h"
#include "destructible.h"
#include "projectile.h"

std::function<Vector2(float)> linear(Vector2 from, Vector2 velocity) {
	return [=](float t) -> Vector2 { return Vector2Add(from, Vector2Scale(velocity, t)); };
}

std::function<Vector2(float)> accelerated(Vector2 from, Vector2 velocity, Vector2 acceleration) {
	return [=](float t) -> Vector2 { return Vector2Add(from, Vector2Add(Vector2Scale(velocity, t), Vector2Scale(acceleration, 0.5f * t * t))); };
}

std::function<Vector2(float)> horizontal_bounce(Vector2 from, Vector2 velocity) {
	bool nvx = velocity.x < 0;
	return [=](float t) -> Vector2 {
		float horizontal_distance = from.x + velocity.x * t - PLAYING_FIELD_TOP_LEFT.x;
		int bounces = int(fabsf(horizontal_distance / PLAYING_FIELD_RECT.width)) + (nvx and horizontal_distance < 0) ? 1 : 0;
		float local_distance = fmodf(fmodf(horizontal_distance, PLAYING_FIELD_RECT.width) + PLAYING_FIELD_RECT.width, PLAYING_FIELD_RECT.width);

		return Vector2{ PLAYING_FIELD_TOP_LEFT.x + (bounces % 2 == 0 ? local_distance : PLAYING_FIELD_RECT.width - local_distance), from.y + velocity.y * t };
	};
}

std::function<Vector2(float)> quadratic_bezier(Vector2 from, Vector2 to, Vector2 control) {
	return [=](float u) -> Vector2 {
		float omu = 1.0f - u;
		return Vector2{ from.x * omu * omu + control.x * omu * u + to.x * u * u, from.y * omu * omu + control.y * omu * u + to.y * u * u };
	};
}

inline std::vector<Projectile> get_basic_player_shot(Vector2 position) {
	return { Projectile{ BASIC_PLAYER_SHOT_RADIUS, RED, linear(position, Vector2{ 0.0f, -BASIC_PLAYER_SHOT_SPEED })} };
}

inline std::vector<Projectile> get_split_player_shot(Vector2 position) {
	return { Projectile{ SPLIT_PLAYER_SHOT_RADIUS, RED, horizontal_bounce(position, Vector2{ -SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED })},
		Projectile{ SPLIT_PLAYER_SHOT_RADIUS, RED, horizontal_bounce(position, Vector2{ SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED })} };
}

inline Destructible get_popcorn_0(Vector2 position, Vector2 direction, std::shared_ptr<Emitter> emitter = nullptr) {
	return Destructible{ POPCORN_0_RADIUS, BLUE, linear(position, Vector2Scale(Vector2Normalize(direction), POPCORN_0_SPEED)), POPCORN_0_HEALTH, emitter };
}

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

struct DestructibleSpawner {
	float cd_timer;
	Destructible destructible_to_spawn;
	//Emitter emitter_to_spawn;
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

inline Vector2 get_input_vector(int neg_x, int pos_x, int neg_y, int pos_y) {
	Vector2 input_direction;

	if (IsKeyDown(neg_x) and IsKeyDown(pos_x)) { input_direction.x = 0.0f; }
	else if (IsKeyDown(neg_x)) { input_direction.x = -1.0f; }
	else if (IsKeyDown(pos_x)) { input_direction.x = 1.0f; }
	else { input_direction.x = 0.0f; }

	if (IsKeyDown(neg_y) and IsKeyDown(pos_y)) { input_direction.y = 0.0f; }
	else if (IsKeyDown(neg_y)) { input_direction.y = -1.0f; }
	else if (IsKeyDown(pos_y)) { input_direction.y = 1.0f; }
	else { input_direction.y = 0.0f; }

	return Vector2Normalize(input_direction);
}

struct Player {
	Vector2 position;
	Vector2 velocity{ 0.0f, 0.0f };

	float shoot_timer = 0.0f;

	std::vector<Projectile> Update(float delta) {
		bool is_focus = IsKeyDown(KEY_LEFT_SHIFT);

		velocity = Vector2Scale(get_input_vector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN), is_focus ? PLAYER_FOCUS_SPEED : PLAYER_NORMAL_SPEED);
		position = Vector2Add(position, Vector2Scale(velocity, delta));
		position = Vector2Clamp(position, PLAYING_FIELD_TOP_LEFT, PLAYING_FIELD_BOTTOM_RIGHT);

		if (IsKeyDown(KEY_Z) and shoot_timer <= 0.0f) {
			shoot_timer = PLAYER_SHOOT_CD;
			if (is_focus) {
				return get_split_player_shot(position);
			}
			return get_basic_player_shot(position);
		}
		else if (shoot_timer > 0.0f) {
			shoot_timer -= delta;
		}

		return {};
	}

	void Draw(void) {
		DrawCircleV(position, PLAYER_HITBOX_RADIUS, PINK);
	}
};

struct Game {
	Player player{};
	std::queue<DestructibleSpawner> spawn_queue;
	std::list<Projectile> player_projectile_list;
	std::list<Projectile> enemy_projectile_list;
	std::list<Destructible> destructible_list;
	std::list<std::shared_ptr<Emitter>> emitter_list;

	Game(std::queue<DestructibleSpawner> level_spawn_queue) {
		player.position = PLAYER_INITIAL_VECTOR;
		spawn_queue = level_spawn_queue;
	}

	void Dead(void) {
		
	}

	void Update(float delta) {
		if (not spawn_queue.empty() and spawn_queue.front().Update(delta)) {
			destructible_list.push_back(spawn_queue.front().destructible_to_spawn);
			if (destructible_list.back().contained_emitter != nullptr) {
				emitter_list.push_back(destructible_list.back().contained_emitter);
				destructible_list.back().contained_emitter->it = std::prev(emitter_list.end());
			}
			spawn_queue.pop();
		}
		std::vector<std::list<Projectile>::iterator> pp_to_remove;
		for (std::list<Projectile>::iterator it = player_projectile_list.begin(); it != player_projectile_list.end(); it = std::next(it)) {
			if (it->Update(delta)) {
				pp_to_remove.push_back(it);
			}
			else {
				for (std::list<Destructible>::iterator d_it = destructible_list.begin(); d_it != destructible_list.end(); d_it = std::next(d_it)) {
					if (it->Collide(d_it->GetPosition(), d_it->radius)) {
						if (d_it->Hurt()) {
							if (d_it->contained_emitter != nullptr) {
								emitter_list.erase(d_it->contained_emitter->it);
							}
							destructible_list.erase(d_it);
						}
						pp_to_remove.push_back(it);
						break;
					}
				}
			}
		}
		for (Projectile& pp : player.Update(delta)) {
			player_projectile_list.push_back(pp);
		}
		for (std::shared_ptr<Emitter> emitter : emitter_list) {
			for (Projectile& ep : emitter->Update(delta, player.position)) {
				enemy_projectile_list.push_back(ep);
			}
		}
		for (std::list<Projectile>::iterator it : pp_to_remove) {
			player_projectile_list.erase(it);
		}
		std::vector<std::list<Projectile>::iterator> ep_to_remove;
		for (std::list<Projectile>::iterator it = enemy_projectile_list.begin(); it != enemy_projectile_list.end(); it = std::next(it)) {
			if (it->Update(delta)) {
				ep_to_remove.push_back(it);
			}
			else if (it->Collide(player.position, PLAYER_HITBOX_RADIUS)) {
			}
		}
		for (std::list<Projectile>::iterator it : ep_to_remove) {
			enemy_projectile_list.erase(it);
		}
		std::vector<std::list<Destructible>::iterator> destructible_to_remove;
		for (std::list<Destructible>::iterator it = destructible_list.begin(); it != destructible_list.end(); it = std::next(it)) {
			if (it->Update(delta)) {
				destructible_to_remove.push_back(it);
			}

		}
		for (std::list<Destructible>::iterator it : destructible_to_remove) {
			if (it->contained_emitter != nullptr) {
				emitter_list.erase(it->contained_emitter->it);
			}
			destructible_list.erase(it);
		}
	}

	void Draw(void) {
		for (std::list<Destructible>::iterator it = destructible_list.begin(); it != destructible_list.end(); it = std::next(it)) {
			it->Draw();
		}

		for (std::list<Projectile>::iterator it = player_projectile_list.begin(); it != player_projectile_list.end(); it = std::next(it)) {
			it->Draw();
		}

		for (std::list<Projectile>::iterator it = enemy_projectile_list.begin(); it != enemy_projectile_list.end(); it = std::next(it)) {
			it->Draw();
		}

		player.Draw();
	}
};

int main(void)
{
	std::queue<DestructibleSpawner> test_level;
	for (int i = 0; i < 20; i++) {
		Emitter sae = Emitter{ single_aimed_shot(0.5f) };
		test_level.push(
			DestructibleSpawner{
				0.4f,
				get_popcorn_0(Vector2Add(PLAYING_FIELD_TOP_LEFT, Vector2{0.0f, 2 * TILE_HEIGHT}), Vector2{100.0f, float(GetRandomValue(0, 20))}, std::make_shared<Emitter>(Emitter{ single_aimed_shot(0.5f) }))
			}
		);
	}

	Game game(test_level);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "borno");

	SetTargetFPS(120);
	while (!WindowShouldClose())
	{
		float frame_delta = GetFrameTime();

		game.Update(frame_delta);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawRectangleRec(PLAYING_FIELD_RECT, LIGHTGRAY);

		game.Draw();

		DrawFPS(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 20);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}