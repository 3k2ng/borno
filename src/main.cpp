#include "raylib.h"
#include "raymath.h"

#include <list>
#include <vector>
#include <optional>
#include <queue>

#include <iostream>

#include "config.h"

#include "emitter.h"
#include "destructible.h"
#include "projectile.h"

struct Spawner {
	float timer;
	Destructible destructible_to_spawn;
	//Emitter emitter_to_spawn;
	bool Update(float delta) {
		if (timer <= 0.0f) {
			return true;
		}
		if (timer > 0.0f) {
			timer -= delta;
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
		std::vector<Projectile> pp_to_insert;

		bool is_focus = IsKeyDown(KEY_LEFT_SHIFT);

		velocity = Vector2Scale(get_input_vector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN), is_focus ? PLAYER_FOCUS_SPEED : PLAYER_NORMAL_SPEED);
		position = Vector2Add(position, Vector2Scale(velocity, delta));
		position = Vector2Clamp(position, PLAYING_FIELD_TOP_LEFT, PLAYING_FIELD_BOTTOM_RIGHT);

		if (IsKeyDown(KEY_Z) and shoot_timer <= 0.0f) {
			if (is_focus) {
				pp_to_insert.push_back(Projectile{ position, Vector2{ SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED }, Projectile::Type::SPLIT_PLAYER_SHOT });
				pp_to_insert.push_back(Projectile{ position, Vector2{ -SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED }, Projectile::Type::SPLIT_PLAYER_SHOT });
			}
			else {
				pp_to_insert.push_back(Projectile{ position, Vector2{0.0f, -BASIC_PLAYER_SHOT_SPEED}, Projectile::Type::BASIC_PLAYER_SHOT });
			}
			shoot_timer = PLAYER_SHOOT_CD;
		}
		else if (shoot_timer > 0.0f) {
			shoot_timer -= delta;
		}

		return pp_to_insert;
	}

	void Draw(void) {
		DrawCircleV(position, PLAYER_HITBOX_RADIUS, PINK);
	}
};

struct Game {
	Player player{};
	std::queue<Spawner> spawn_queue;
	std::list<Projectile> player_projectile_list;
	std::list<Projectile> enemy_projectile_list;
	std::list<Destructible> destructible_list;
	std::list<Emitter> emitter_list;

	Game(std::queue<Spawner> level_spawn_queue) {
		player.position = PLAYER_INITIAL_VECTOR;
		spawn_queue = level_spawn_queue;
	}

	void Dead(void) {
		
	}

	void Update(float delta) {
		if (not spawn_queue.empty() and spawn_queue.front().Update(delta)) {
			destructible_list.push_back(spawn_queue.front().destructible_to_spawn);
			spawn_queue.pop();
		}

		for (Projectile& pp : player.Update(delta)) {
			player_projectile_list.push_back(pp);
		}
		for (Emitter& emitter : emitter_list) {
			for (Projectile& ep : emitter.Update(delta, player.position)) {
				enemy_projectile_list.push_back(ep);
			}
		}
		std::vector<std::list<Projectile>::iterator> pp_to_remove;
		for (std::list<Projectile>::iterator it = player_projectile_list.begin(); it != player_projectile_list.end(); it = std::next(it)) {
			if (it->Update(delta)) {
				pp_to_remove.push_back(it);
			}
			else {
				float damage_to_deal = 0.0f;
				switch (it->type)
				{
				case(Projectile::Type::BASIC_PLAYER_SHOT):
					damage_to_deal = BASIC_PLAYER_SHOT_DAMAGE;
					break;
				case(Projectile::Type::SPLIT_PLAYER_SHOT):
					damage_to_deal = SPLIT_PLAYER_SHOT_DAMAGE;
					break;
				default:
					break;
				}

				for (std::list<Destructible>::iterator d_it = destructible_list.begin(); d_it != destructible_list.end(); d_it = std::next(d_it)) {
					if (it->Collide(d_it->position, d_it->GetRadius())) {
						if (d_it->Hurt(damage_to_deal)) {
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
	std::queue<Spawner> test_level;

	for (int i = 0; i < 20; i++) {
		test_level.push(
			Spawner{
				0.4f,
				Destructible(
					Vector2Add(PLAYING_FIELD_TOP_LEFT, Vector2{0.0f, 2 * TILE_HEIGHT}),
					Destructible::Type::POPCORN_0,
					nullptr,
					Vector2Scale(Vector2Normalize(Vector2{100.0f, float(GetRandomValue(0, 20))}), POPCORN_0_SPEED)
				)
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