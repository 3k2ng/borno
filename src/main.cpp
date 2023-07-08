#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <list>
#include <vector>
#include <optional>

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;

constexpr int NUM_COLUMN = 8;
constexpr int NUM_ROW = 16;

constexpr Vector2 CENTER = Vector2{
	0.5f * float(SCREEN_WIDTH) / float(NUM_COLUMN),
	0.5f * float(SCREEN_HEIGHT) / float(NUM_ROW)
};
constexpr float BRICK_OFFSET_X = float(SCREEN_WIDTH) / float(NUM_COLUMN);
constexpr float BRICK_OFFSET_Y = float(SCREEN_HEIGHT) / float(NUM_ROW);

constexpr float BRICK_WIDTH = BRICK_OFFSET_X - 10.0f;
constexpr float BRICK_HEIGHT = BRICK_OFFSET_Y - 10.0f;


struct Brick {
	Vector2 position;
	float width;
	float height;

	void Draw(Color color) {
		float half_width = 0.5f * width;
		float half_height = 0.5f * height;

		rlBegin(RL_QUADS);

		rlColor4ub(color.r, color.g, color.b, color.a);

		rlVertex2f(position.x - half_width, position.y - half_height);
		rlVertex2f(position.x - half_width, position.y + half_height);
		rlVertex2f(position.x + half_width, position.y + half_height);
		rlVertex2f(position.x + half_width, position.y - half_height);

		rlEnd();
	}
};

struct Ball {
	Vector2 position;
	Vector2 velocity;
	float radius;

	void Update(float delta) {
		position = Vector2Add(position, Vector2Scale(velocity, delta));

		if (position.x - radius < 0 or position.x + radius > SCREEN_WIDTH) {
			velocity.x *= -1;
		}

		if (position.y - radius < 0 or position.y + radius > SCREEN_HEIGHT) {
			velocity.y *= -1;
		}
	}

	std::optional<Vector2> Collide(Brick brick) {
		Vector2 distance = Vector2Subtract(position, brick.position);

		float distance_x = fabsf(distance.x);
		float distance_y = fabsf(distance.y);

		float half_brick_width = 0.5f * brick.width;
		float half_brick_height = 0.5f * brick.height;

		if (distance_x > half_brick_width + radius) { return std::nullopt; }
		if (distance_y > half_brick_height + radius) { return std::nullopt; }

		if (distance_x <= half_brick_width) { return distance; }
		if (distance_y <= half_brick_height) { return distance; }

		float diff_x = distance_x - half_brick_width;
		float diff_y = distance_y - half_brick_height;

		if (diff_x * diff_x + diff_y * diff_y <= radius * radius) return distance;

		return std::nullopt;
	}

	void Draw(Color color) {
		rlBegin(RL_QUADS);

		rlColor4ub(color.r, color.g, color.b, color.a);

		rlVertex2f(position.x - radius, position.y - radius);
		rlVertex2f(position.x - radius, position.y + radius);
		rlVertex2f(position.x + radius, position.y + radius);
		rlVertex2f(position.x + radius, position.y - radius);

		rlEnd();
	}
};

int main(void)
{
	Ball ball{
		Vector2{ SCREEN_WIDTH / 2, 100 },
		Vector2{ 200, 200 },
		20
	};

	std::vector<Brick> bricks;

	for (int i = 0; i < NUM_COLUMN; i++) {
		for (int j = NUM_ROW / 2; j < NUM_ROW; j++) {
			bricks.push_back(Brick{
					Vector2{ i * BRICK_OFFSET_X + CENTER.x, j * BRICK_OFFSET_Y + CENTER.y },
					BRICK_WIDTH,
					BRICK_HEIGHT
				});
		}
	}

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "borno");

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
		ball.Update(GetFrameTime());

		BeginDrawing();
		ClearBackground(RAYWHITE);

		ball.Draw(RED);

		for (auto it = bricks.begin(); it != bricks.end(); it = std::next(it)) {
			it->Draw(LIGHTGRAY);
		}

		for (auto it = bricks.begin(); it != bricks.end(); it = std::next(it)) {
			std::optional<Vector2> collision_opt = ball.Collide(*it);
			if (collision_opt) {
				Vector2 collision_normal = Vector2Normalize(collision_opt.value());
				ball.velocity = Vector2Scale(collision_normal, Vector2Length(ball.velocity));
				bricks.erase(it);
				break;
			}
		}

		DrawText("BORNO", 10, 10, 100, RED);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}