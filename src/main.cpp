#include "raylib.h"
#include "raymath.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "borno");

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("BORNO", 0, 0, 100, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}