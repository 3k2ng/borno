#pragma once

#include "raylib.h"
#include "raymath.h"

#include <functional>

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

std::function<Vector2(float)> quadratic_bezier(Vector2 from, Vector2 to, Vector2 control, float travel_time = 1.0f) {
	return [=](float u) -> Vector2 {
		u /= travel_time;
		return Vector2Lerp(Vector2Lerp(from, control, u), Vector2Lerp(control, to, u), u);
	};
}

std::function<Vector2(float)> quadratic_bezier_with_pause(Vector2 from, Vector2 to, Vector2 control, float travel_time = 1.0f, float pause_at = 0.5f, float pause_for = 1.0f) {
	return [=](float t) -> Vector2 {
		float u;

		if (t <= pause_at) {
			u = t / travel_time;
		}
		else if (t > pause_at and t <= pause_at + pause_for) {
			u = pause_at / travel_time;
		}
		else {
			u = (t - pause_for) / travel_time;
		}

		return Vector2Lerp(Vector2Lerp(from, control, u), Vector2Lerp(control, to, u), u);
	};
}