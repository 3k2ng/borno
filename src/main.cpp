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

constexpr float GAP_X = 20.f;
constexpr float GAP_Y = 20.f;

constexpr float BRICK_WIDTH = BRICK_OFFSET_X - 0.5f * GAP_X;
constexpr float BRICK_HEIGHT = BRICK_OFFSET_Y - 0.5f * GAP_Y;


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

struct Player {
	enum class Collision { NONE, UP, DOWN, LEFT, RIGHT };
	Brick player_brick;
	Vector2 velocity;
	bool is_on_floor = false;

	void Update(float delta, const std::vector<Brick>& others) {
		bool c_up = false;
		bool c_down = false;
		bool c_left = false;
		bool c_right = false;
		for (std::vector<Brick>::const_iterator it = others.begin(); it != others.end(); it = std::next(it)) {
			switch (Collide(*it))
			{
			case Collision::UP:
				c_up = true;
				break;
			case Collision::DOWN:
				c_down = true;
				break;
			case Collision::LEFT:
				c_left = true;
				break;
			case Collision::RIGHT:
				c_right = true;
				break;
			default:
				break;
			}
			if (c_up and c_down and c_left and c_right) {
				break;
			}
		}

		if (c_down) { is_on_floor = true; }
		else { is_on_floor = false; }

		if (IsKeyDown(KEY_LEFT) and not c_left) {
			velocity.x = -100;
		}
		else if (IsKeyDown(KEY_RIGHT) and not c_right) {
			velocity.x = 100;
		}
		else {
			velocity.x = 0;
		}

		if (IsKeyPressed(KEY_UP) and is_on_floor) {
			velocity.y = -600;
		}
		else if (not is_on_floor) {
			velocity.y += 1000 * delta;
		}
		else if (is_on_floor) {
			velocity.y = 0;
		}

		player_brick.position = Vector2Add(player_brick.position, Vector2Scale(velocity, delta));
	}

	Collision Collide(const Brick& brick) {
		Vector2 distance = Vector2Subtract(brick.position, player_brick.position);

		float distance_x = fabsf(distance.x);
		float distance_y = fabsf(distance.y);

		float half_player_brick_width = 0.5f * player_brick.width;
		float half_player_brick_height = 0.5f * player_brick.height;

		float half_brick_width = 0.5f * brick.width;
		float half_brick_height = 0.5f * brick.height;

		if (distance_x > half_player_brick_width + half_brick_width) { return Collision::NONE; }
		if (distance_y > half_player_brick_height + half_brick_height) { return Collision::NONE; }

		if (distance_x / distance_y < (half_player_brick_width + half_brick_width) / (half_player_brick_height + half_brick_height)) {
			if (distance.y < 0) {
				return Collision::UP;
			}
			return Collision::DOWN;
		}

		if (distance.x < 0) {
			return Collision::LEFT;
		}
		return Collision::RIGHT;
	}

	void Draw(Color color) {
		player_brick.Draw(color);
	}
};

struct Ball {
	Vector2 position;
	Vector2 velocity;
	float radius;

	void Update(float delta) {
		position = Vector2Add(position, Vector2Scale(velocity, delta));

		if (position.x - radius < 0 and velocity.x < 0) {
			velocity.x *= -1;
		}
		else if (position.x + radius > SCREEN_WIDTH and velocity.x > 0) {
			velocity.x *= -1;
		}

		if (position.y - radius < 0 and velocity.y < 0) {
			velocity.y *= -1;
		}
		else if (position.y + radius > SCREEN_HEIGHT and velocity.y > 0) {
			velocity.y *= -1;
		}
	}

	std::optional<Vector2> Collide(const Brick& brick) {
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
		Vector2{ 800, 800 },
		20
	};

	std::vector<Brick> bricks;

	Player player{
		Brick{ Vector2{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 100 }, BRICK_WIDTH, BRICK_HEIGHT },
		Vector2Zero()
	};

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

		for (auto it = bricks.begin(); it != bricks.end(); it = std::next(it)) {
			std::optional<Vector2> collision_opt = ball.Collide(*it);
			if (collision_opt) {
				Vector2 collision_normal = Vector2Normalize(collision_opt.value());
				ball.velocity = Vector2Scale(collision_normal, Vector2Length(ball.velocity));
				bricks.erase(it);
				break;
			}
		}

		player.Update(GetFrameTime(), bricks);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		ball.Draw(RED);

		for (auto it = bricks.begin(); it != bricks.end(); it = std::next(it)) {
			it->Draw(LIGHTGRAY);
		}

		player.Draw(BLUE);

		DrawText("BORNO", 10, 10, 100, RED);
		DrawFPS(0, 0);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}