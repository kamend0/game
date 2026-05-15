#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 480
#define DEFAULT_FONT_SIZE 64

// Audio callback to generate a sine wave in real-time
void DumbAlarmCallback(void *bufferData, unsigned int frames) {
    float *buffer = (float *)bufferData;
    static float phase = 0.0f;
    static float sweepTime = 0.0f;

    const float highFreq = 880.0f;
    const float lowFreq = 440.0f;
    const float sweepDuration = 0.5f; // seconds per high->low sweep

    for (unsigned int i = 0; i < frames; i++) {
        float t = sweepTime / sweepDuration;
        float frequency = highFreq + (lowFreq - highFreq) * t;

        buffer[i] = sinf(phase);

        phase += 2.0f * PI * frequency / 44100.0f;
        if (phase > 2.0f * PI)
            phase -= 2.0f * PI;

        sweepTime += 1.0f / 44100.0f;
        if (sweepTime >= sweepDuration)
            sweepTime -= sweepDuration;
    }
}

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

void updatePlayerPos(Rectangle *player) {
    if (IsKeyDown(KEY_W)) {
        (*player).y -= 5;
    };
    if (IsKeyDown(KEY_A)) {
        (*player).x -= 5;
    };
    if (IsKeyDown(KEY_S)) {
        (*player).y += 5;
    };
    if (IsKeyDown(KEY_D)) {
        (*player).x += 5;
    };
}

void showStatusMessage(bool isColliding) {
    char *allGood = "You're good!";
    char *instructions = "Use WASD to move";
    char *colliding = "DANGER DANGER DANGER DANGER DANGER DANGER DANGER";
    char *whywhy =
        "OH FUCK NO WHY DID YOU DO THAT OH SHIT WHY WHY WHY WHY WHY "
        "WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY WHY";
    // char *chosenMessage = NULL;

    unsigned int messageSize = 0;

    if (!isColliding) {
        messageSize = DEFAULT_FONT_SIZE;

        DrawText(
            allGood,
            ((GetScreenWidth() - MeasureText(allGood, messageSize)) * 0.05f),
            ((GetScreenHeight() - messageSize) * 0.05f), messageSize, GRAY);
        DrawText(instructions,
                 ((GetScreenWidth() - MeasureText(instructions, 24)) * 0.04f),
                 ((GetScreenHeight() - messageSize) * 0.2f), 24, BLACK);
    } else {
        messageSize = 128;
        DrawText(colliding, ((GetScreenWidth()) * 0.02f),
                 ((GetScreenHeight() - messageSize) * 0.02f), messageSize, RED);
        DrawText(whywhy, ((GetScreenWidth()) * 0.02f),
                 ((GetScreenHeight() - 24) * 0.25f), 24, RED);
    }
}

int main(void) {
    InitWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "No Tuch");
    InitAudioDevice();

    SetTargetFPS(60);

    Rectangle player = {(float)DEFAULT_WIDTH / 2, (float)DEFAULT_HEIGHT / 2, 50,
                        50};
    Rectangle enemy = {(float)DEFAULT_WIDTH * 0.85f,
                       (float)DEFAULT_HEIGHT * 0.75f, 150, 150};

    bool areColliding = false;
    char *warningMessage = "DO NOT TOUCH!";

    // Audio stream
    AudioStream stream = LoadAudioStream(44100, 32, 1);
    SetAudioStreamCallback(stream, DumbAlarmCallback);

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsKeyPressed(KEY_F)) {
            f_ToggleFullscreen();
        }

        updatePlayerPos(&player);
        DrawRectangleRec(player, BLUE);
        DrawRectangleRec(enemy, RED);

        // Collision handling

        DrawText(warningMessage, enemy.x + (MeasureText(warningMessage, 8) / 4),
                 enemy.y - 10, 8, RED);

        areColliding = CheckCollisionRecs(player, enemy);
        showStatusMessage(areColliding);

        if (areColliding) {
            PlayAudioStream(stream);
        } else {
            StopAudioStream(stream);
        }

        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    StopAudioStream(stream);
    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

