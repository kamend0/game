#include "raylib.h"
#include <stdio.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 480

void f_ToggleFullscreen() {
    if (!IsWindowFullscreen()) {
        // Switch TO fullscreen
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        // Switch BACK to windowed
        ToggleFullscreen();
        SetWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
}

int main(void) {
    const char *message = "Hello!";
    const int fontSize = 64;

    InitWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "WE ARE IN!!!");
    ToggleFullscreen();

    Vector2 pos = {(float)DEFAULT_WIDTH, (float)DEFAULT_HEIGHT};

    const int messageWidth = MeasureText(message, fontSize);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsKeyPressed(KEY_F)) {
            f_ToggleFullscreen();
        }

        ClearBackground(RAYWHITE);
        DrawText(message, (GetScreenWidth() - messageWidth) / 2.0f,
                 (GetScreenHeight() - fontSize) / 2.0f, fontSize, GRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

