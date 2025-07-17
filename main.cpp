#include "raylib.h"
#include<raymath.h>
#include<iostream>
#include <map>
#include <cstdlib>
#include <ctime>

// Board size
const int BOARD_SIZE = 10;
const int TILE_SIZE = 100;
const int windowWidth = 1920;
const int windowHeight = 1080;
const int BOARD_PIXEL = BOARD_SIZE * TILE_SIZE;
const int boardX = (windowWidth - BOARD_PIXEL) / 2;
const int boardY = (windowHeight - BOARD_PIXEL) / 2;

// Snakes & Ladders positions
std::map<int, int> snakes = {
    {16, 6}, {47, 26}, {49, 11}, {56, 53}, //{62, 19}, {64, 60},
    {87, 24}, {93, 73}, //{95, 75}, {98, 78}
};

std::map<int, int> ladders = {
    {1, 38}, {4, 14}, {9, 31}, //{21, 42}, {28, 84},
    {36, 44}, {51, 67}, //{71, 91}, {80, 100}
};

// Get 2D position from board index (1–100)
Vector2 GetTilePosition(int index)
{
    int row = (index - 1) / BOARD_SIZE;
    int col = (index - 1) % BOARD_SIZE;
    if (row % 2 == 1) col = BOARD_SIZE - 1 - col;
    return Vector2{ (float)(col * TILE_SIZE), (float)(BOARD_PIXEL - (row + 1) * TILE_SIZE) };
}

// Draw a stretched texture between two tiles
/*void DrawStretchedTexture(Vector2 start, Vector2 end, Texture2D texture, float thickness)
{
    Vector2 dir = Vector2Subtract(end, start);
    float length = Vector2Length(dir);
    float angle = atan2f(dir.y, dir.x) * RAD2DEG;

    Vector2 center = Vector2Lerp(start, end, 0.5f);
    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { center.x, center.y, length, thickness };
    Vector2 origin = { length / 2, thickness / 2 };

    DrawTexturePro(texture, source, dest, origin, angle, WHITE);
}
    */

void DrawRepeatedTexture(Vector2 start, Vector2 end, Texture2D texture, float thickness)
{
    Vector2 dir = Vector2Subtract(end, start);
    float length = Vector2Length(dir);
    float angle = atan2f(dir.y, dir.x) * RAD2DEG;

    int numTiles = (int)(length / texture.width);
    float remainder = length - numTiles * texture.width;

    Vector2 pos = start;
    for (int i = 0; i < numTiles; i++) {
        Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle dest = { pos.x + (texture.width/2.0f) * cosf(angle*DEG2RAD),
                           pos.y + (texture.width/2.0f) * sinf(angle*DEG2RAD),
                           (float)texture.width, thickness };
        Vector2 origin = { texture.width/2.0f, thickness/2.0f };
        DrawTexturePro(texture, source, dest, origin, angle, WHITE);
        pos.x += texture.width * cosf(angle*DEG2RAD);
        pos.y += texture.width * sinf(angle*DEG2RAD);
    }
    // Draw the remainder
    if (remainder > 1) {
        Rectangle source = { 0, 0, remainder, (float)texture.height };
        Rectangle dest = { pos.x + (remainder/2.0f) * cosf(angle*DEG2RAD),
                           pos.y + (remainder/2.0f) * sinf(angle*DEG2RAD),
                           remainder, thickness };
        Vector2 origin = { remainder/2.0f, thickness/2.0f };
        DrawTexturePro(texture, source, dest, origin, angle, WHITE);
    }
}

// Draw lives as textures at the top left
void DrawLives(int lives, Texture2D lifeTexture) {
    int iconSize = 100;
    int spacing = 10;
    int x0 = 100, y0 = 100;
    for (int i = 0; i < lives; i++) {
        Rectangle source = { 0, 0, (float)lifeTexture.width, (float)lifeTexture.height };
        Rectangle dest = { (float)(x0 + i * (iconSize + spacing)), (float)y0, (float)iconSize, (float)iconSize };
        Vector2 origin = { 0, 0 };
        DrawTexturePro(lifeTexture, source, dest, origin, 0, WHITE);
    }
}

// Draw a replay button and return true if clicked
bool DrawReplayButton(int screenWidth, int screenHeight) {
    int btnWidth = 180, btnHeight = 50;
    int x = (screenWidth - btnWidth) / 2;
    int y = (screenHeight - btnHeight) / 2;
    Rectangle btn = { (float)x, (float)y, (float)btnWidth, (float)btnHeight };
    bool hovered = CheckCollisionPointRec(GetMousePosition(), btn);
    DrawRectangleRec(btn, hovered ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(btn, 2, DARKGRAY);
    DrawText("Replay", x + 40, y + 12, 28, hovered ? RED : MAROON);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

int main()
{
    InitWindow(windowWidth, windowHeight, "Snakes & Ladders 2D - raylib");

    Texture2D snakeTexture = LoadTexture("pngwing.com.png");
    Texture2D ladderTexture = LoadTexture("pngwing(1).com.png");
    Texture2D lifeTexture = LoadTexture("life.png");
    Texture2D diceTextures[6];
    for (int i = 0; i < 6; i++) {
        char filename[20];
        sprintf(filename, "dice%d.png", i+1);
        diceTextures[i] = LoadTexture(filename);
    }
    int playerTile = 1;
    int lives = 3;
    bool gameOver = false;
    bool won = false;

    int currentDice = 0;           // The dice face to display (0-5)
    bool rolling = false;          // Is the dice currently rolling?
    float rollTime = 0.0f;         // How long has the dice been rolling?
    float rollDuration = 0.8f;     // Total roll animation time (seconds)
    float rollFrameTime = 0.08f;   // Time between dice face changes
    float rollFrameCounter = 0.0f; // Time accumulator for frame change
    int rolledValue = 1;           // The final dice value (1-6)

    int moveSteps = 0;           // How many steps left to move
    int moveTargetTile = 1;      // The tile to move to
    float moveDelay = 0.2f;      // Delay between steps (seconds)
    float moveTimer = 0.0f;      // Timer for step delay
    bool moving = false;         // Is the player currently moving?
    int lastDiceValue = 1;       // Store the last dice value

    SetTargetFPS(60);
    srand((unsigned int)time(NULL));

    while (!WindowShouldClose())
    {
        if (!rolling && !moving && !gameOver && !won && IsKeyPressed(KEY_SPACE)) {
            rolling = true;
            rollTime = 0.0f;
            rollFrameCounter = 0.0f;
        }

        if (rolling) {
            rollTime += GetFrameTime();
            rollFrameCounter += GetFrameTime();
            if (rollFrameCounter >= rollFrameTime) {
                currentDice = GetRandomValue(0, 5);
                rollFrameCounter = 0.0f;
            }
            if (rollTime >= rollDuration) {
                rolling = false;
                lastDiceValue = GetRandomValue(1, 6);
                currentDice = lastDiceValue - 1;
                moveSteps = lastDiceValue;
                moveTargetTile = playerTile + moveSteps;
                if (moveTargetTile > 100) moveTargetTile = 100;
                moving = true;
                moveTimer = 0.0f;
            }
        }

        if (moving && !gameOver && !won) {
            moveTimer += GetFrameTime();
            if (moveTimer >= moveDelay && playerTile < moveTargetTile) {
                playerTile++;
                moveTimer = 0.0f;
                if (playerTile == moveTargetTile) {
                    // After movement, check for snakes/ladders
                    bool fellInSnake = false;
                    if (snakes.count(playerTile)) {
                        playerTile = snakes[playerTile];
                        fellInSnake = true;
                    }
                    if (ladders.count(playerTile)) playerTile = ladders[playerTile];
                    if (fellInSnake) {
                        lives--;
                        if (lives <= 0) {
                            lives = 0;
                            gameOver = true;
                        }
                    }
                    if (playerTile == 100) {
                        won = true;
                    }
                    moving = false;
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw board
        for (int i = 1; i <= 100; i++)
        {
            Vector2 pos = GetTilePosition(i);
            pos.x += boardX;
            pos.y += boardY;
            Color color = (i % 2 == 0) ? SKYBLUE : LIGHTGRAY;
            DrawRectangle((int)pos.x, (int)pos.y, TILE_SIZE, TILE_SIZE, color);
            DrawRectangleLines((int)pos.x, (int)pos.y, TILE_SIZE, TILE_SIZE, DARKGRAY);
            DrawText(TextFormat("%d", i), (int)pos.x + 4, (int)pos.y + 4, 16, DARKBLUE);
        }

        /* Draw snakes
        for (const auto& s : snakes) {
            Vector2 start = Vector2Add(GetTilePosition(s.first), Vector2{TILE_SIZE/2.0f, TILE_SIZE/2.0f});
            Vector2 end = Vector2Add(GetTilePosition(s.second), Vector2{TILE_SIZE/2.0f, TILE_SIZE/2.0f});
            start.x += boardX; start.y += boardY;
            end.x += boardX; end.y += boardY;
            DrawRepeatedTexture(start, end, snakeTexture, 24);
        }
        // Draw ladders
        for (const auto& l : ladders) {
            Vector2 start = Vector2Add(GetTilePosition(l.first), Vector2{TILE_SIZE/2.0f, TILE_SIZE/2.0f});
            Vector2 end = Vector2Add(GetTilePosition(l.second), Vector2{TILE_SIZE/2.0f, TILE_SIZE/2.0f});
            start.x += boardX; start.y += boardY;
            end.x += boardX; end.y += boardY;
            DrawRepeatedTexture(start, end, ladderTexture, 16);
        }
        */

        // Draw lives
        DrawLives(lives, lifeTexture);

        // Draw player
        Vector2 playerPos = Vector2Add(GetTilePosition(playerTile), Vector2{TILE_SIZE/2.0f, TILE_SIZE/2.0f});
        playerPos.x += boardX;
        playerPos.y += boardY;
        DrawCircleV(playerPos, TILE_SIZE/4.0f, RED);

        // Draw dice animation (to the right of the board, 1/4th board size)
        int diceSize = BOARD_PIXEL / 4;
        int diceX = boardX + BOARD_PIXEL + diceSize / 2 + 20;
        int diceY = boardY + (BOARD_PIXEL - diceSize) / 2;
        
        if (rolling || !gameOver) {
        Vector2 pos = { (float)diceX, (float)diceY };
        float rotation = 0.0f; // no rotation
        float scale = (float)diceSize / diceTextures[currentDice].width;
        DrawTextureEx(diceTextures[currentDice], pos, rotation, scale, WHITE);
        }

        // Win screen
        if (won) {
            DrawRectangle(0, 0, windowWidth, windowHeight, Fade(BLACK, 0.5f));
            DrawText("YOU WON!", windowWidth/2 - 120, windowHeight/2 - 100, 48, GREEN);
            if (DrawReplayButton(windowWidth, windowHeight)) {
                playerTile = 1;
                lives = 3;
                gameOver = false;
                won = false;
            }
        }

        if (gameOver && lives == 0) {
            // Blur effect: draw a semi-transparent rectangle over the whole window
            DrawRectangle(0, 0, windowWidth, windowHeight, Fade(BLACK, 0.5f));
            DrawText("Game Over!", windowWidth/2 - 80, windowHeight/2 - 100, 36, MAROON);
            if (DrawReplayButton(windowWidth, windowHeight)) {
                playerTile = 1;
                lives = 3;
                gameOver = false;
                won = false;
            }
        }

        DrawText("Press [SPACE] to roll dice", boardX, boardY + BOARD_PIXEL + 10, 24, DARKGRAY);
        DrawText(TextFormat("Current Tile: %d", playerTile), boardX, boardY + BOARD_PIXEL + 40, 24, DARKBLUE);

        EndDrawing();
    }
    UnloadTexture(snakeTexture);
    UnloadTexture(ladderTexture);
    UnloadTexture(lifeTexture);
    for (int i = 0; i < 6; i++) UnloadTexture(diceTextures[i]);

    CloseWindow();
    return 0;
}
