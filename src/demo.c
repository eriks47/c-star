#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <stdlib.h>

#include "a_star.h"

#define MAP_SIZE 10

typedef int map_t[MAP_SIZE][MAP_SIZE];

enum {
    EMPTY,

    PLAYER,
    ENEMY,

    OBSTACLES,
    WATER,
    WALL,
};

bool is_traversable(int id) {
    return id < OBSTACLES;
}

void init_window() {
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(600, 600, "A* Pathfinding Algorithm");
}

void render(map_t map) {
    BeginDrawing();
    ClearBackground(BLACK);

    int cell_size = 600 / MAP_SIZE;

    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int k = 0; k < MAP_SIZE; ++k) {
            Rectangle rect = {i * cell_size, k * cell_size, cell_size, cell_size};

            Color color;
            switch (map[i][k]) {
            case EMPTY: color = WHITE; break;
            case PLAYER: color = BLUE; break;
            case ENEMY: color = RED; break;
            default: color = BLACK;
            }

            DrawRectangleRec(rect, color);
            DrawRectangleLinesEx(rect, 2.0, DARKGRAY);
        }
    }

    EndDrawing();
}

int main(void) {
    map_t map;
    memset(map, EMPTY, sizeof(int) * MAP_SIZE * MAP_SIZE);

    map[1][1] = PLAYER;
    map[9][9] = ENEMY;

    coords_t player = {1, 1};
    coords_t enemy = {9, 9};

    init_window();

    double last_step = 0; // To "slow down" the algorithm
    bool paused = false;
    while (!WindowShouldClose()) {
        render(map);

        if (IsKeyReleased(KEY_SPACE))
            paused = !paused;

        if (paused && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            int x = GetMouseX();
            int y = GetMouseY();
            int cx = x / (600 / MAP_SIZE);
            int cy = y / (600 / MAP_SIZE);

            // I have no idea why we need to do (cx, cy) instead of (cy, cx)
            // when indexing the map but it is the case for some reason. The
            // cx and cy values are correct, I tested it. If you do (cy, cx)
            // it works improperly.
            if (map[cx][cy] == EMPTY) {
                map[cx][cy] = WALL;
            }
        }

        if (GetTime() - last_step > 0.5 && !paused) {
            last_step = GetTime();

            coords_t pos = compute_next_position(&map[0][0], MAP_SIZE, MAP_SIZE, player, enemy, &is_traversable);
            map[player.y][player.x] = EMPTY;
            player.x = pos.x;
            player.y = pos.y;
            map[player.y][player.x] = PLAYER;
        }
    }
}
