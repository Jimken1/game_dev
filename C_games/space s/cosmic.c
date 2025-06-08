#include "raylib.h"
#include "cosmic.h"
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool active;
    Color color;
} Bullet;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    int health;
    bool active;
    float shootTimer;
    int type; // 0 = basic, 1 = fast, 2 = heavy
    Color color;
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float life;
    Color color;
    bool active;
} Particle;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    int type; // 0 = rapid fire, 1 = shield, 2 = triple shot
    bool active;
    float rotation;
    Color color;
} PowerUp;

typedef struct {
    Vector2 position;
    float radius;
    int health;
    int maxHealth;
    float speed;
    bool shieldActive;
    float shieldTimer;
    int fireMode; // 0 = normal, 1 = rapid, 2 = triple
    float fireModeTimer;
} Player;

Player player;
Bullet bullets[MAX_BULLETS];
Bullet enemyBullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];
PowerUp powerUps[MAX_POWERUPS];

int score = 0;
int wave = 1;
int enemiesKilled = 0;
float gameTime = 0;
bool gameOver = false;
bool paused = false;



void InitGame(void) {
    // Initialize player
    player.position = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT - 100};
    player.radius = 20;
    player.health = 100;
    player.maxHealth = 100;
    player.speed = 5.0f;
    player.shieldActive = false;
    player.shieldTimer = 0;
    player.fireMode = 0;
    player.fireModeTimer = 0;
    
    // Initialize arrays
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
        enemyBullets[i].active = false;
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        powerUps[i].active = false;
    }
    
    // Initialize stars
    for (int i = 0; i < 100; i++) {
        stars[i].x = GetRandomValue(0, SCREEN_WIDTH);
        stars[i].y = GetRandomValue(0, SCREEN_HEIGHT);
        starSpeeds[i] = GetRandomValue(1, 4) / 10.0f;
    }
    
    // Reset game variables
    score = 0;
    wave = 1;
    enemiesKilled = 0;
    gameTime = 0;
    gameOver = false;
    paused = false;
}

void UpdateGame(void) {
    gameTime += GetFrameTime();
    
    UpdatePlayer();
    UpdateBullets();
    UpdateEnemies();
    UpdateParticles();
    UpdatePowerUps();
    CheckCollisions();
    
    // Spawn enemies based on wave
    if (GetRandomValue(0, 100) < 2 + wave) {
        SpawnEnemy();
    }
    
    // Spawn power-ups occasionally
    if (GetRandomValue(0, 1000) < 5) {
        SpawnPowerUp((Vector2){GetRandomValue(50, SCREEN_WIDTH-50), -20});
    }
    
    // Check if player is dead
    if (player.health <= 0) {
        gameOver = true;
    }
    
    // Update wave
    if (enemiesKilled >= wave * 5) {
        wave++;
        enemiesKilled = 0;
    }
}

void UpdatePlayer(void) {
    // Player movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player.position.x -= player.speed;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player.position.x += player.speed;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player.position.y -= player.speed;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        player.position.y += player.speed;
    }
    
    // Keep player on screen
    if (player.position.x < player.radius) player.position.x = player.radius;
    if (player.position.x > SCREEN_WIDTH - player.radius) player.position.x = SCREEN_WIDTH - player.radius;
    if (player.position.y < player.radius) player.position.y = player.radius;
    if (player.position.y > SCREEN_HEIGHT - player.radius) player.position.y = SCREEN_HEIGHT - player.radius;
    
    // Player shooting
    static float shootTimer = 0;
    shootTimer += GetFrameTime();
    
    float fireRate = (player.fireMode == 1) ? 0.1f : 0.2f;
    
    if ((IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && shootTimer >= fireRate) {
        // Find inactive bullet
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].position = player.position;
                bullets[i].velocity = (Vector2){0, -10};
                bullets[i].radius = 3;
                bullets[i].active = true;
                bullets[i].color = YELLOW;
                
                if (player.fireMode == 2) { // Triple shot
                    // Left bullet
                    if (i + 1 < MAX_BULLETS && !bullets[i + 1].active) {
                        bullets[i + 1].position = (Vector2){player.position.x - 15, player.position.y};
                        bullets[i + 1].velocity = (Vector2){-2, -10};
                        bullets[i + 1].radius = 3;
                        bullets[i + 1].active = true;
                        bullets[i + 1].color = YELLOW;
                    }
                    // Right bullet
                    if (i + 2 < MAX_BULLETS && !bullets[i + 2].active) {
                        bullets[i + 2].position = (Vector2){player.position.x + 15, player.position.y};
                        bullets[i + 2].velocity = (Vector2){2, -10};
                        bullets[i + 2].radius = 3;
                        bullets[i + 2].active = true;
                        bullets[i + 2].color = YELLOW;
                    }
                }
                break;
            }
        }
        shootTimer = 0;
    }
    
    // Update shield
    if (player.shieldActive) {
        player.shieldTimer -= GetFrameTime();
        if (player.shieldTimer <= 0) {
            player.shieldActive = false;
        }
    }
    
    // Update fire mode
    if (player.fireModeTimer > 0) {
        player.fireModeTimer -= GetFrameTime();
        if (player.fireModeTimer <= 0) {
            player.fireMode = 0;
        }
    }
}

void UpdateBullets(void) {
    // Update player bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].position.x += bullets[i].velocity.x;
            bullets[i].position.y += bullets[i].velocity.y;
            
            // Remove bullets that go off screen
            if (bullets[i].position.y < 0 || bullets[i].position.x < 0 || bullets[i].position.x > SCREEN_WIDTH) {
                bullets[i].active = false;
            }
        }
    }
    
    // Update enemy bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (enemyBullets[i].active) {
            enemyBullets[i].position.x += enemyBullets[i].velocity.x;
            enemyBullets[i].position.y += enemyBullets[i].velocity.y;
            
            // Remove bullets that go off screen
            if (enemyBullets[i].position.y > SCREEN_HEIGHT || enemyBullets[i].position.x < 0 || enemyBullets[i].position.x > SCREEN_WIDTH) {
                enemyBullets[i].active = false;
            }
        }
    }
}

void UpdateEnemies(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].position.x += enemies[i].velocity.x;
            enemies[i].position.y += enemies[i].velocity.y;
            
            // Enemy AI - simple movement patterns
            if (enemies[i].type == 0) { // Basic enemy
                enemies[i].velocity.y = 2;
            } else if (enemies[i].type == 1) { // Fast enemy
                enemies[i].velocity.y = 4;
                enemies[i].velocity.x = sin(gameTime * 3) * 2;
            } else if (enemies[i].type == 2) { // Heavy enemy
                enemies[i].velocity.y = 1;
            }
            
            // Enemy shooting
            enemies[i].shootTimer += GetFrameTime();
            if (enemies[i].shootTimer >= 1.0f + GetRandomValue(0, 200) / 100.0f) {
                // Find inactive enemy bullet
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!enemyBullets[j].active) {
                        enemyBullets[j].position = enemies[i].position;
                        
                        // Aim towards player
                        Vector2 direction = {player.position.x - enemies[i].position.x, 
                                           player.position.y - enemies[i].position.y};
                        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                        direction.x /= length;
                        direction.y /= length;
                        
                        enemyBullets[j].velocity = (Vector2){direction.x * 5, direction.y * 5};
                        enemyBullets[j].radius = 4;
                        enemyBullets[j].active = true;
                        enemyBullets[j].color = RED;
                        break;
                    }
                }
                enemies[i].shootTimer = 0;
            }
            
            // Remove enemies that go off screen
            if (enemies[i].position.y > SCREEN_HEIGHT + 50) {
                enemies[i].active = false;
            }
        }
    }
}

void UpdateParticles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;
            particles[i].life -= GetFrameTime();
            
            // Fade out particle
            particles[i].color.a = (unsigned char)(255 * particles[i].life);
            
            if (particles[i].life <= 0) {
                particles[i].active = false;
            }
        }
    }
}

void UpdatePowerUps(void) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerUps[i].active) {
            powerUps[i].position.x += powerUps[i].velocity.x;
            powerUps[i].position.y += powerUps[i].velocity.y;
            powerUps[i].rotation += 2.0f;
            
            // Remove power-ups that go off screen
            if (powerUps[i].position.y > SCREEN_HEIGHT + 50) {
                powerUps[i].active = false;
            }
        }
    }
}

void CheckCollisions(void) {
    // Player bullets vs enemies
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].active) {
                    if (CheckCollisionCircles(bullets[i].position, bullets[i].radius,
                                            enemies[j].position, enemies[j].radius)) {
                        bullets[i].active = false;
                        enemies[j].health--;
                        
                        // Spawn hit particles
                        SpawnParticles(enemies[j].position, enemies[j].color, 3);
                        
                        if (enemies[j].health <= 0) {
                            // Enemy destroyed
                            SpawnParticles(enemies[j].position, enemies[j].color, 15);
                            enemies[j].active = false;
                            score += (enemies[j].type + 1) * 10;
                            enemiesKilled++;
                            
                            // Chance to spawn power-up
                            if (GetRandomValue(0, 100) < 20) {
                                SpawnPowerUp(enemies[j].position);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Enemy bullets vs player
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (enemyBullets[i].active) {
            if (CheckCollisionCircles(enemyBullets[i].position, enemyBullets[i].radius,
                                    player.position, player.radius)) {
                enemyBullets[i].active = false;
                
                if (!player.shieldActive) {
                    player.health -= 10;
                    SpawnParticles(player.position, RED, 8);
                } else {
                    SpawnParticles(player.position, BLUE, 5);
                }
            }
        }
    }
    
    // Enemies vs player
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (CheckCollisionCircles(enemies[i].position, enemies[i].radius,
                                    player.position, player.radius)) {
                enemies[i].active = false;
                
                if (!player.shieldActive) {
                    player.health -= 20;
                    SpawnParticles(player.position, RED, 12);
                } else {
                    SpawnParticles(player.position, BLUE, 8);
                }
                
                SpawnParticles(enemies[i].position, enemies[i].color, 10);
            }
        }
    }
    
    // Power-ups vs player
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerUps[i].active) {
            if (CheckCollisionCircles(powerUps[i].position, powerUps[i].radius,
                                    player.position, player.radius)) {
                powerUps[i].active = false;
                SpawnParticles(powerUps[i].position, powerUps[i].color, 10);
                
                // Apply power-up effect
                if (powerUps[i].type == 0) { // Rapid fire
                    player.fireMode = 1;
                    player.fireModeTimer = 10.0f;
                } else if (powerUps[i].type == 1) { // Shield
                    player.shieldActive = true;
                    player.shieldTimer = 15.0f;
                } else if (powerUps[i].type == 2) { // Triple shot
                    player.fireMode = 2;
                    player.fireModeTimer = 8.0f;
                }
            }
        }
    }
}

void SpawnEnemy(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].position.x = GetRandomValue(50, SCREEN_WIDTH - 50);
            enemies[i].position.y = -30;
            enemies[i].velocity = (Vector2){0, 2};
            enemies[i].active = true;
            enemies[i].shootTimer = 0;
            
            // Different enemy types based on wave
            int enemyType = GetRandomValue(0, 2);
            if (wave < 3) enemyType = 0; // Only basic enemies early on
            
            enemies[i].type = enemyType;
            
            if (enemyType == 0) { // Basic
                enemies[i].radius = 15;
                enemies[i].health = 1;
                enemies[i].color = RED;
            } else if (enemyType == 1) { // Fast
                enemies[i].radius = 12;
                enemies[i].health = 1;
                enemies[i].color = ORANGE;
            } else { // Heavy
                enemies[i].radius = 20;
                enemies[i].health = 3;
                enemies[i].color = PURPLE;
            }
            break;
        }
    }
}

void SpawnParticles(Vector2 position, Color color, int count) {
    for (int i = 0; i < count && i < MAX_PARTICLES; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                particles[j].position = position;
                particles[j].velocity.x = GetRandomValue(-100, 100) / 10.0f;
                particles[j].velocity.y = GetRandomValue(-100, 100) / 10.0f;
                particles[j].radius = GetRandomValue(1, 3);
                particles[j].life = GetRandomValue(5, 15) / 10.0f;
                particles[j].color = color;
                particles[j].active = true;
                break;
            }
        }
    }
}

void SpawnPowerUp(Vector2 position) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].active) {
            powerUps[i].position = position;
            powerUps[i].velocity = (Vector2){0, 2};
            powerUps[i].radius = 12;
            powerUps[i].type = GetRandomValue(0, 2);
            powerUps[i].active = true;
            powerUps[i].rotation = 0;
            
            if (powerUps[i].type == 0) powerUps[i].color = GREEN;      // Rapid fire
            else if (powerUps[i].type == 1) powerUps[i].color = BLUE;  // Shield
            else powerUps[i].color = YELLOW;                           // Triple shot
            break;
        }
    }
}

void DrawStars(void) {
    for (int i = 0; i < 100; i++) {
        stars[i].y += starSpeeds[i];
        if (stars[i].y > SCREEN_HEIGHT) {
            stars[i].y = 0;
            stars[i].x = GetRandomValue(0, SCREEN_WIDTH);
        }
        
        Color starColor = WHITE;
        starColor.a = (unsigned char)(255 * starSpeeds[i]);
        DrawPixel((int)stars[i].x, (int)stars[i].y, starColor);
    }
}

void DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    if (!gameOver) {
        DrawStars();
        
        // Draw player
        Color playerColor = WHITE;
        if (player.shieldActive) {
            // Draw shield effect
            DrawCircleLines((int)player.position.x, (int)player.position.y, 
                          player.radius + 10, BLUE);
            DrawCircleLines((int)player.position.x, (int)player.position.y, 
                          player.radius + 15, SKYBLUE);
        }
        DrawCircleV(player.position, player.radius, playerColor);
        DrawCircleLines((int)player.position.x, (int)player.position.y, player.radius, DARKGRAY);
        
        // Draw bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
            }
            if (enemyBullets[i].active) {
                DrawCircleV(enemyBullets[i].position, enemyBullets[i].radius, enemyBullets[i].color);
            }
        }
        
        // Draw enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                DrawCircleV(enemies[i].position, enemies[i].radius, enemies[i].color);
                DrawCircleLines((int)enemies[i].position.x, (int)enemies[i].position.y, 
                              enemies[i].radius, DARKGRAY);
            }
        }
        
        // Draw particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                DrawCircleV(particles[i].position, particles[i].radius, particles[i].color);
            }
        }
        
        // Draw power-ups
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (powerUps[i].active) {
                DrawCircleV(powerUps[i].position, powerUps[i].radius, powerUps[i].color);
                DrawCircleLines((int)powerUps[i].position.x, (int)powerUps[i].position.y, 
                              powerUps[i].radius, WHITE);
                
                // Draw power-up symbol
                if (powerUps[i].type == 0) DrawText("R", (int)powerUps[i].position.x - 5, (int)powerUps[i].position.y - 8, 16, BLACK);
                else if (powerUps[i].type == 1) DrawText("S", (int)powerUps[i].position.x - 5, (int)powerUps[i].position.y - 8, 16, BLACK);
                else DrawText("3", (int)powerUps[i].position.x - 5, (int)powerUps[i].position.y - 8, 16, BLACK);
            }
        }
        
        DrawUI();
        
        if (paused) {
            DrawText("PAUSED", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2, 40, WHITE);
            DrawText("Press P to continue", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50, 20, GRAY);
        }
    } else {
        // Game over screen
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, 40, RED);
        DrawText(TextFormat("Final Score: %d", score), SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2, 20, WHITE);
        DrawText(TextFormat("Wave Reached: %d", wave), SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 30, 20, WHITE);
        DrawText("Press R to restart", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 80, 20, GRAY);
    }
    
    EndDrawing();
}

void DrawUI(void) {
    // Health bar
    DrawRectangle(10, 10, 200, 20, DARKGRAY);
    DrawRectangle(10, 10, (int)(200 * (player.health / (float)player.maxHealth)), 20, RED);
    DrawText("HEALTH", 10, 35, 20, WHITE);
    
    // Score
    DrawText(TextFormat("SCORE: %d", score), 10, 70, 20, WHITE);
    
    // Wave
    DrawText(TextFormat("WAVE: %d", wave), 10, 95, 20, WHITE);
    
    // Power-up status
    int yOffset = 120;
    if (player.shieldActive) {
        DrawText(TextFormat("SHIELD: %.1fs", player.shieldTimer), 10, yOffset, 20, BLUE);
        yOffset += 25;
    }
    if (player.fireMode == 1) {
        DrawText(TextFormat("RAPID FIRE: %.1fs", player.fireModeTimer), 10, yOffset, 20, GREEN);
        yOffset += 25;
    }
    if (player.fireMode == 2) {
        DrawText(TextFormat("TRIPLE SHOT: %.1fs", player.fireModeTimer), 10, yOffset, 20, YELLOW);
    }
    
    // Controls
    DrawText("WASD/Arrow Keys: Move", SCREEN_WIDTH - 200, SCREEN_HEIGHT - 80, 16, GRAY);
    DrawText("Space/Click: Shoot", SCREEN_WIDTH - 200, SCREEN_HEIGHT - 60, 16, GRAY);
    DrawText("P: Pause", SCREEN_WIDTH - 200, SCREEN_HEIGHT - 40, 16, GRAY);
    DrawText("R: Restart (when dead)", SCREEN_WIDTH - 200, SCREEN_HEIGHT - 20, 16, GRAY);
}