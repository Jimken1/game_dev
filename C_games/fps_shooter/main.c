#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fps.h"
#include "fps.c"


int main() {
    InitWindow(1024, 768, "Awesome FPS Game");
    SetTargetFPS(60);
    DisableCursor();
    
    InitMap();
    InitPlayer();
    InitCamera();
    
    // Initialize arrays
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
    
    SpawnWave();
    
    while (!WindowShouldClose()) {
        gameTime += GetFrameTime();
        
        if (!gameOver) {
            if (player.health <= 0) {
                gameOver = true;
            }
            
            UpdatePlayer();
            UpdateEnemies();
            UpdateBullets();
            UpdateParticles();
            
            // Check if wave is complete
            bool waveComplete = true;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    waveComplete = false;
                    break;
                }
            }
            
            if (waveComplete) {
                waveTimer += GetFrameTime();
                if (waveTimer > 3.0f) {
                    wave++;
                    waveTimer = 0;
                    SpawnWave();
                }
            }
            
            // Update camera
            camera.position = player.pos;
            camera.target = Vector3Add(player.pos, player.front);
        } else {
            if (IsKeyPressed(KEY_R)) {
                // Restart game
                gameOver = false;
                InitPlayer();
                score = 0;
                wave = 1;
                enemiesKilled = 0;
                waveTimer = 0;
                gameTime = 0;
                
                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
                for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
                for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
                
                SpawnWave();
            }
        }
        
        BeginDrawing();
        ClearBackground(SKYBLUE);
        
        BeginMode3D(camera);
        DrawMap();
        DrawEnemies();
        DrawBullets();
        DrawParticles();
        EndMode3D();
        
        DrawHUD();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}