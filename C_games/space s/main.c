#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "cosmic.h"
#include "cosmic.c"

// Global game variables

// Function prototypes

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cosmic Defender");
    SetTargetFPS(60);
    
    // Initialize random seed
    srand(time(NULL));
    
    InitGame();
    
    while (!WindowShouldClose()) {
        if (!gameOver && !paused) {
            UpdateGame();
        }
        
        // Handle pause
        if (IsKeyPressed(KEY_P) && !gameOver) {
            paused = !paused;
        }
        
        // Restart game
        if (IsKeyPressed(KEY_R) && gameOver) {
            InitGame();
        }
        
        DrawGame();
    }
    
    CloseWindow();
    return 0;
}

