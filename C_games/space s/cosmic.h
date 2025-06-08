#include "raylib.h"

#define MAX_BULLETS 75
#define MAX_ENEMIES 25
#define MAX_PARTICLES 150
#define MAX_POWERUPS 10
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800


void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void SpawnEnemy(void);
void SpawnParticles(Vector2 position, Color color, int count);
void SpawnPowerUp(Vector2 position);
void UpdatePlayer(void);
void UpdateBullets(void);
void UpdateEnemies(void);
void UpdateParticles(void);
void UpdatePowerUps(void);
void CheckCollisions(void);
void DrawUI(void);
void DrawStars(void);

// Star field for background
Vector2 stars[100];
float starSpeeds[100];

