#include "raylib.h"
#include "raymath.h"
#include <map>
#include <cstdlib>
#include <ctime>

// Board size
const int BOARD_SIZE = 10;
const float TILE_SIZE = 2.0f;

// Snakes & Ladders positions
std::map<int, int> snakes = {
    {16, 6}, {47, 26}, {49, 11}, {56, 53}, {62, 19}, {64, 60},
    {87, 24}, {93, 73}, {95, 75}, {98, 78}
};

std::map<int, int> ladders = {
    {1, 38}, {4, 14}, {9, 31}, {21, 42}, {28, 84},
    {36, 44}, {51, 67}, {71, 91}, {80, 100}
};

// Get 3D position from board index (1–100)
Vector3 GetTilePosition(int index)
{
    int row = (index - 1) / BOARD_SIZE;
    int col = (index - 1) % BOARD_SIZE;
    if (row % 2 == 1) col = BOARD_SIZE - 1 - col;
    return Vector3{ col * TILE_SIZE, 0.0f, row * TILE_SIZE };
}

int main()
{
    InitWindow(800, 600, "Snakes & Ladders 3D - raylib");
    InitAudioDevice(); // Optional

    Camera camera = { 0 };
    camera.position = { 10.0f, 20.0f, 40.0f };
    camera.target = { 10.0f, -0.0f, 10.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int playerTile = 1;

    SetTargetFPS(60);
    srand(static_cast<unsigned int>(time(NULL)));


    while (!WindowShouldClose())
    {
        //UpdateCamera(&camera, CAMERA_ORBITAL);

        if (IsKeyPressed(KEY_SPACE))
        {
            int dice = GetRandomValue(1, 6);
            playerTile += dice;
            if (playerTile > 100) playerTile = 100;

            if (snakes.count(playerTile)) playerTile = snakes[playerTile];
            if (ladders.count(playerTile)) playerTile = ladders[playerTile];
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Draw board
        for (int i = 1; i <= 100; i++)
        {
            Vector3 pos = GetTilePosition(i);
            Color color = (i % 2 == 0) ? RED : LIGHTGRAY;

            DrawCube(pos, TILE_SIZE, 0.1f, TILE_SIZE, color);
            DrawCubeWires(pos, TILE_SIZE, 0.1f, TILE_SIZE, DARKGRAY);
        }

        // Draw player
        Vector3 playerPos = GetTilePosition(playerTile);
        playerPos.y += 0.5f;
        DrawSphere(playerPos, 0.5f, BLUE);

        EndMode3D();

        DrawText("Press [SPACE] to roll dice", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Current Tile: %d", playerTile), 10, 40, 20, DARKBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
