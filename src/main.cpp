#include "raylib.h"
#include "raymath.h"

#include <list>
#include <vector>
#include <optional>

#include <iostream>

#include "config.h"

#include "emitter.h"
#include "destructible.h"
#include "projectile.h"

struct Spawner {
	float timer;
	Destructible to_spawn;
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
	Vector2 velocity;

	float shoot_timer = 0.0f;

	std::vector<Projectile> Update(float delta) {
		std::vector<Projectile> pp_to_insert;

		bool is_focus = IsKeyDown(KEY_LEFT_SHIFT);

		velocity = Vector2Scale(get_input_vector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN), is_focus ? PLAYER_FOCUS_SPEED : PLAYER_NORMAL_SPEED);
		position = Vector2Add(position, Vector2Scale(velocity, delta));
		position = Vector2Clamp(position, PLAYING_FIELD_TOP_LEFT, PLAYING_FIELD_BOTTOM_RIGHT);

		if (IsKeyDown(KEY_Z) and shoot_timer <= 0.0f) {
			if (is_focus) {
				pp_to_insert.push_back(Projectile{ position, Vector2{SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED}, Projectile::Type::SPLIT_PLAYER_SHOT });
				pp_to_insert.push_back(Projectile{ position, Vector2{-SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED}, Projectile::Type::SPLIT_PLAYER_SHOT });
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
	std::list<Projectile> player_projectile_list;
	std::list<Projectile> enemy_projectile_list;
	std::list<Destructible> destructible_list;
	std::list<Emitter> emitter_list;

	Game() {
		player = Player{ PLAYER_INITIAL_VECTOR, Vector2Zero() };
		emitter_list.emplace_back();
		emitter_list.back().it = std::prev(emitter_list.end());
		emitter_list.back().position = Vector2{
				(PLAYING_FIELD_OFFSET_HORIZONTAL_TILES + PLAYING_FIELD_HORIZONTAL_TILES / 2) * TILE_WIDTH,
				(PLAYING_FIELD_OFFSET_VERTICAL_TILES + PLAYING_FIELD_VERTICAL_TILES / 2) * TILE_HEIGHT
		};
		emitter_list.back().type = Emitter::Type::TESTING_EMITTER;
		destructible_list.emplace_back(
			Vector2{
				(PLAYING_FIELD_OFFSET_HORIZONTAL_TILES + PLAYING_FIELD_HORIZONTAL_TILES / 2) * TILE_WIDTH,
				(PLAYING_FIELD_OFFSET_VERTICAL_TILES + PLAYING_FIELD_VERTICAL_TILES / 2) * TILE_HEIGHT
			},
			Destructible::Type::TESTING_DUMMY,
			&emitter_list.back()
		);
	}

	void Update(float delta) {
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
				for (std::list<Destructible>::iterator d_it = destructible_list.begin(); d_it != destructible_list.end(); d_it = std::next(d_it)) {
					if (it->Collide(d_it->position, d_it->GetRadius())) {
						if (d_it->Hurt(0.0f)) {
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
	Game game;

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