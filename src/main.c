#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 480
#define DEFAULT_FONT_SIZE 64
#define DEFAULT_PLAYER_SPEED 8

typedef struct Player {
    Rectangle collision;
    Rectangle display;
    float rotation;
    Color color;
} Player;

void initPlayer(Player *player, int x, int y, int width, int height,
                float rotation, Color color) {
    // Player is initialized with some initial position. Its display rect is
    // offset from its collision rect by 1/2 its width and height to match what
    // is rendered by DrawRectanglePro
    *player =
        (Player){.collision = (Rectangle){x - (width / 2), y - (height / 2),
                                          width, height},
                 .display = (Rectangle){x, y, width, height},
                 .rotation = rotation,
                 .color = color};
}

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

int clamp(int min, int max, int value) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }
    return value;
}

void updatePlayerPos(Player *player) {
    // Because collision and display are now disjointed, they are subject to
    // different bounds; display is to the bottom-right of the collision square,
    // so we have to make sure to allow it to exceed the bounds of the screen a
    // bit to stay in sync
    int display_min_x = ((*player).collision.width / 2);
    int display_min_y = ((*player).collision.width / 2);
    int display_max_x = GetScreenWidth() - (0.5 * (*player).display.width);
    int display_max_y = GetScreenHeight() - (0.5 * (*player).display.height);

    // NOTE: Only works because width and height are divisible by 2. Otherwise,
    // we're one pixel off.
    int collision_min_x = 0;
    int collision_min_y = 0;
    int collision_max_x = display_max_x - ((*player).collision.width / 2);
    int collision_max_y = display_max_y - ((*player).collision.height / 2);

    if (IsKeyDown(KEY_W)) {
        (*player).collision.y =
            clamp(collision_min_y, collision_max_y,
                  (*player).collision.y - DEFAULT_PLAYER_SPEED);
        (*player).display.y = clamp(display_min_y, display_max_y,
                                    (*player).display.y - DEFAULT_PLAYER_SPEED);
    };
    if (IsKeyDown(KEY_A)) {
        (*player).collision.x =
            clamp(collision_min_x, collision_max_x,
                  (*player).collision.x - DEFAULT_PLAYER_SPEED);
        (*player).display.x = clamp(display_min_x, display_max_x,
                                    (*player).display.x - DEFAULT_PLAYER_SPEED);
    };
    if (IsKeyDown(KEY_S)) {
        (*player).collision.y =
            clamp(collision_min_y, collision_max_y,
                  (*player).collision.y + DEFAULT_PLAYER_SPEED);
        (*player).display.y = clamp(display_min_y, display_max_y,
                                    (*player).display.y + DEFAULT_PLAYER_SPEED);
    };
    if (IsKeyDown(KEY_D)) {
        (*player).collision.x =
            clamp(collision_min_x, collision_max_x,
                  (*player).collision.x + DEFAULT_PLAYER_SPEED);
        (*player).display.x = clamp(display_min_x, display_max_x,
                                    (*player).display.x + DEFAULT_PLAYER_SPEED);
    };
}

void updatePlayerRot(float *rotation) {
    if (IsKeyPressed(KEY_SPACE)) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            *rotation -= 45.0f;
        } else {
            *rotation += 45.0f;
        }
    }
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

    Player player;
    initPlayer(&player, (float)DEFAULT_WIDTH / 2, (float)DEFAULT_HEIGHT / 2, 50,
               50, 0.0f, BLUE);
    // Rectangle player = {(float)DEFAULT_WIDTH / 2, (float)DEFAULT_HEIGHT / 2,
    // 50, 50};
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

        DrawText(warningMessage, enemy.x + (MeasureText(warningMessage, 8) / 4),
                 enemy.y - 10, 8, RED);

        updatePlayerPos(&player);
        updatePlayerRot(&player.rotation);

        DrawRectangleRec(enemy, RED);
        // To rotate about its center, the Rect is shifted, so its positioning
        // logic is messed up. Need a solution to this.
        DrawRectanglePro(player.display,
                         (Vector2){player.display.width / 2.0f,
                                   player.display.height / 2.0f},
                         player.rotation, player.color);

        // Collision handling
        areColliding = CheckCollisionRecs(player.collision, enemy);
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

