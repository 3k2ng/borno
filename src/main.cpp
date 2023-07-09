#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <list>
#include <vector>
#include <optional>

#include "config.h"

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

	void Update(float delta) {
		velocity = Vector2Multiply(get_input_vector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN), PLAYER_SPEED_VECTOR);
		position = Vector2Add(position, Vector2Scale(velocity, delta));
		position = Vector2Clamp(position, PLAYING_FIELD_TOP_LEFT, PLAYING_FIELD_BOTTOM_RIGHT);
	}

	void Draw(Color color) {
		DrawCircleV(position, PLAYER_HITBOX_RADIUS, color);
	}
};

int main(void)
{
	Player player{ PLAYER_INITIAL_VECTOR, Vector2Zero() };

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "borno");

	SetTargetFPS(120);
	while (!WindowShouldClose())
	{
		float frame_delta = GetFrameTime();

		player.Update(frame_delta);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawRectangleRec(PLAYING_FIELD_RECT, LIGHTGRAY);

		player.Draw(RED);

		DrawFPS(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 20);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}