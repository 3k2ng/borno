#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <list>
#include <vector>
#include <optional>

#include <iostream>

#include "config.h"

constexpr float TESTING_DUMMY_RADIUS = 20.f;

struct Destructible {
	Vector2 position;
	Vector2 velocity;
	enum class Type {
		TESTING_DUMMY
	};
	Type type;
	void Draw(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			DrawCircleV(position, TESTING_DUMMY_RADIUS, BLUE);
			break;
		default:
			break;
		}
	}
	bool Hurt(float damage_value) {
		return false;
	}
	float GetRadius(void) {
		switch (type)
		{
		case Destructible::Type::TESTING_DUMMY:
			return TESTING_DUMMY_RADIUS;
		default:
			return 0.0f;
		}
	}
};

constexpr float BASIC_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float BASIC_PLAYER_SHOT_SPEED = 600.0f;

constexpr float SPLIT_PLAYER_SHOT_RADIUS = 8.0f;
constexpr float SPLIT_PLAYER_SHOT_V_SPEED = 600.0f;
constexpr float SPLIT_PLAYER_SHOT_H_SPEED = 600.0f;
struct Projectile {
	Vector2 position;
	Vector2 velocity;
	enum class Type {
		BASIC_PLAYER_SHOT,
		SPLIT_PLAYER_SHOT
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
			return Vector2DistanceSqr(position, c_position) < (BASIC_PLAYER_SHOT_RADIUS + c_radius) * (BASIC_PLAYER_SHOT_RADIUS + c_radius);
			break;
		case Projectile::Type::SPLIT_PLAYER_SHOT:
			return Vector2DistanceSqr(position, c_position) < (SPLIT_PLAYER_SHOT_RADIUS + c_radius) * (SPLIT_PLAYER_SHOT_RADIUS + c_radius);
			break;
		default:
			break;
		}
		return false;
	}

	void Draw(void) {
		switch (type)
		{
		case Type::BASIC_PLAYER_SHOT:
		{
			DrawCircleV(position, BASIC_PLAYER_SHOT_RADIUS, RED);
		}
		case Type::SPLIT_PLAYER_SHOT:
		{
			DrawCircleV(position, SPLIT_PLAYER_SHOT_RADIUS, RED);
		}
		break;
		default:
			break;
		}
	}
};

struct Emitter {
	std::vector<Projectile> Update(void) {

	}
};

struct Spawner {

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
		std::vector<Projectile> to_insert;

		bool is_focus = IsKeyDown(KEY_LEFT_SHIFT);

		velocity = Vector2Scale(get_input_vector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN), is_focus ? PLAYER_FOCUS_SPEED : PLAYER_NORMAL_SPEED);
		position = Vector2Add(position, Vector2Scale(velocity, delta));
		position = Vector2Clamp(position, PLAYING_FIELD_TOP_LEFT, PLAYING_FIELD_BOTTOM_RIGHT);

		if (IsKeyDown(KEY_Z) and shoot_timer <= 0.0f) {
			if (is_focus) {
				to_insert.push_back(Projectile{ position, Vector2{SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED}, Projectile::Type::SPLIT_PLAYER_SHOT });
				to_insert.push_back(Projectile{ position, Vector2{-SPLIT_PLAYER_SHOT_H_SPEED, -SPLIT_PLAYER_SHOT_V_SPEED}, Projectile::Type::SPLIT_PLAYER_SHOT });
			}
			else {
				to_insert.push_back(Projectile{ position, Vector2{0.0f, -BASIC_PLAYER_SHOT_SPEED}, Projectile::Type::BASIC_PLAYER_SHOT });
			}
			shoot_timer = PLAYER_SHOOT_CD;
		}
		else if (shoot_timer > 0.0f) {
			shoot_timer -= delta;
		}

		return to_insert;
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

	Game() {
		player = Player{ PLAYER_INITIAL_VECTOR, Vector2Zero() };
		destructible_list.push_back(Destructible{
			Vector2{
				(PLAYING_FIELD_OFFSET_HORIZONTAL_TILES + PLAYING_FIELD_HORIZONTAL_TILES / 2) * TILE_WIDTH,
				(PLAYING_FIELD_OFFSET_VERTICAL_TILES + PLAYING_FIELD_VERTICAL_TILES / 2) * TILE_HEIGHT
			},
			Vector2{ 0.0f, 0.0f }
			});
	}

	void Update(float delta) {
		std::vector<Projectile> to_insert = player.Update(delta);
		for (Projectile p_projectile : to_insert) {
			player_projectile_list.push_back(p_projectile);
		}
		std::vector<std::list<Projectile>::iterator> to_remove;
		for (std::list<Projectile>::iterator it = player_projectile_list.begin(); it != player_projectile_list.end(); it = std::next(it)) {
			if (it->Update(delta)) {
				to_remove.push_back(it);
			}
			else {
				for (std::list<Destructible>::iterator d_it = destructible_list.begin(); d_it != destructible_list.end(); d_it = std::next(d_it)) {
					if (it->Collide(d_it->position, d_it->GetRadius())) {
						to_remove.push_back(it);
						break;
					}
				}
			}
		}
		for (std::list<Projectile>::iterator it : to_remove) {
			player_projectile_list.erase(it);
		}
	}
	void Draw() {
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