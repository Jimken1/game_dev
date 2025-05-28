#define PLATFORM_ANDROID
#include "raylib.h"

#define CELL_SIZE 150  // Larger for better touch targeting
#define GRID_OFFSET_X 50
#define GRID_OFFSET_Y 100
#define MAX_TOUCH_POINTS 2
// Game state
int board[3][3] = {0}; // 0=empty, 1=X, 2=O
int currentPlayer = 1;
bool gameOver = false;
int winner = 0;
//textures
Texture2D texX;
Texture2D texO;

Image icon;
// UI elements
Rectangle restartButton = { 120, 400, 160, 60 };

void ResetGame() {
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = 0;
        }
    }
    currentPlayer = 1;
    gameOver = false;
    winner = 0;
}

int CheckWinner() {
    // Check rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][0] == board[i][2]) {
            return board[i][0];
        }
    }
    
    // Check columns
    for (int j = 0; j < 3; j++) {
        if (board[0][j] != 0 && board[0][j] == board[1][j] && board[0][j] == board[2][j]) {
            return board[0][j];
        }
    }
    
    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[0][0] == board[2][2]) {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[0][2] == board[2][0]) {
        return board[0][2];
    }
    
    // Check for draw
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 0) return 0;
        }
    }
    return 3; // Draw
}



void HandleTouchInput() {
    // Check all touch points (supports multi-touch)
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
        if (IsGestureDetected(GESTURE_TAP)) {
            Vector2 touchPos = GetTouchPosition(i);
            
            if (gameOver) {
                // Check restart button
                if (CheckCollisionPointRec(touchPos, restartButton)) {
                    ResetGame();
                }
            }
            else {
                // Check grid cells
                int col = (touchPos.x - GRID_OFFSET_X) / CELL_SIZE;
                int row = (touchPos.y - GRID_OFFSET_Y) / CELL_SIZE;
                
                if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == 0) {
                    board[row][col] = currentPlayer;
                    winner = CheckWinner();
                    
                    if (winner > 0) {
                        gameOver = true;
                    } else {
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    }
                }
            }
        }
    }
}
void DrawBoard() {
    // Draw grid
	
    for (int i = 0; i <= 3; i++) {
        DrawLineEx((Vector2){GRID_OFFSET_X, GRID_OFFSET_Y + i*CELL_SIZE}, 
                  (Vector2){GRID_OFFSET_X + 3*CELL_SIZE, GRID_OFFSET_Y + i*CELL_SIZE}, 
                  4, BLACK);
        DrawLineEx((Vector2){GRID_OFFSET_X + i*CELL_SIZE, GRID_OFFSET_Y}, 
                  (Vector2){GRID_OFFSET_X + i*CELL_SIZE, GRID_OFFSET_Y + 3*CELL_SIZE}, 
                  4, BLACK);
    }
    
    // Draw X's and O's
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int centerX = GRID_OFFSET_X + j*CELL_SIZE + CELL_SIZE/2;
            int centerY = GRID_OFFSET_Y + i*CELL_SIZE + CELL_SIZE/2;
            
            if (board[i][j] == 1) { // X
			Rectangle destRec = {
				centerX - texX.width/4,
				centerY - texX.height/4,
				texX.width/2,
				texX.height/2
			};
              DrawTexturePro(
				texX,
				(Rectangle){0, 0, texX.width, texX.height},
				destRec,
				(Vector2){0, 0},
				0.0f,
				WHITE 
			  );
            }
            else if (board[i][j] == 2) { // O
			
               Rectangle destRec = {
				centerX - texX.width/4,
				centerY - texX.height/4,
				texX.width/2,
				texX.height/2
			};
			DrawTexturePro(
				texO,
				(Rectangle){0, 0, texO.width, texO.height},
				destRec,
				(Vector2){0, 0},
				0.0f,
				WHITE 
			  );
            }
        }
    }
}

int main(void) {
    // Initialize window - use fullscreen on mobile
    InitWindow(0, 0, "Tic-Tac-Toe");

	
    SetTargetFPS(60);
	icon = LoadImage("resources/1021366.ico");
	SetWindowIcon(icon);
	UnloadImage(icon);

	//load textures
	texX = LoadTexture("resources/X6.png");
	texO = LoadTexture("resources/letter_o_PNG68.png");
	

    SetGesturesEnabled(GESTURE_TAP);
    // On Android, we want fullscreen
    #ifdef PLATFORM_ANDROID
        ToggleFullscreen();
    #endif
    
    // Calculate positions based on screen size
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Adjust UI elements for screen size
    restartButton.x = screenWidth/2 - restartButton.width/2;
    restartButton.y = screenHeight - 150;
    
    ResetGame();
    
    while (!WindowShouldClose()) {
        HandleTouchInput();
        
        BeginDrawing();
            ClearBackground(GRAY);
            
            // Draw title
            DrawText("Tic-Tac-Toe", screenWidth/2 - MeasureText("Tic-Tac-Toe", 40)/2, 30, 40, BLACK);
            
            // Draw current player indicator
            const char* turnText = currentPlayer == 1 ? "X's Turn" : "O's Turn";
            if (!gameOver) {
                DrawText(turnText, screenWidth/2 - MeasureText(turnText, 30)/2, 70, 30, BLACK);
            }
            
            DrawBoard();
            
            // Draw game over message
            if (gameOver) {
                const char* resultText;
                Color textColor = BLACK;
                
                if (winner == 1) {
                    resultText = "X Wins!";
                    textColor = RED;
                }
                else if (winner == 2) {
                    resultText = "O Wins!";
                    textColor = BLUE;
                }
                else {
                    resultText = "It's a Draw!";
                }
                
                DrawText(resultText, screenWidth/2 - MeasureText(resultText, 40)/2, 350, 40, textColor);
                
                // Draw restart button
                DrawRectangleRec(restartButton, LIGHTGRAY);
                DrawRectangleLinesEx(restartButton, 3, DARKGRAY);
                DrawText("Play Again", 
                        restartButton.x + restartButton.width/2 - MeasureText("Play Again", 30)/2,
                        restartButton.y + restartButton.height/2 - 15,
                        30, BLACK);
            }
        EndDrawing();
    }
    
	//unload textures
	UnloadTexture(texX);
	UnloadTexture(texO);
    CloseWindow();
    return 0;
}

