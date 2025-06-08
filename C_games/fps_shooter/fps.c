#include "raylib.h"
#include "fps.h"


typedef struct {
    Vector3 pos;
    Vector3 vel;
    float life;
    Color color;
} Particle;

typedef struct {
    Vector3 pos;
    Vector3 vel;
    bool active;
    float damage;
} Bullet;

typedef struct {
    Vector3 pos;
    float health;
    float maxHealth;
    bool active;
    Vector3 target;
    float speed;
    float lastShot;
    Color color;
} Enemy;

typedef struct {
    Vector3 pos;
    Vector3 front;
    Vector3 up;
    Vector3 right;
    float yaw;
    float pitch;
    float health;
    float maxHealth;
    int ammo;
    float lastShot;
} Player;

// Global variables
Player player;
Camera3D camera;
Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
Particle particles[MAX_PARTICLES];
int score = 0;
int wave = 1;
int enemiesKilled = 0;
float waveTimer = 0;
bool gameOver = false;
float gameTime = 0;

// Map data (simple maze)
int map[MAP_SIZE][MAP_SIZE];

void InitMap() {
    // Create a simple maze-like map
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (i == 0 || i == MAP_SIZE-1 || j == 0 || j == MAP_SIZE-1) {
                map[i][j] = 1; // Wall
            } else if ((i % 4 == 0 && j % 4 == 0) || (i % 6 == 0 && j % 3 == 0)) {
                map[i][j] = 1; // Internal walls
            } else {
                map[i][j] = 0; // Empty space
            }
        }
    }
    
    // Ensure spawn area is clear
    for (int i = 1; i < 5; i++) {
        for (int j = 1; j < 5; j++) {
            map[i][j] = 0;
        }
    }
}

void InitPlayer() {
    player.pos = (Vector3){2.5f, 1.0f, 2.5f};
    player.front = (Vector3){0.0f, 0.0f, -1.0f};
    player.up = (Vector3){0.0f, 1.0f, 0.0f};
    player.right = (Vector3){1.0f, 0.0f, 0.0f};
    player.yaw = -90.0f;
    player.pitch = 0.0f;
    player.health = 100.0f;
    player.maxHealth = 100.0f;
    player.ammo = 30;
    player.lastShot = 0;
}

void InitCamera() {
    camera.position = player.pos;
    camera.target = Vector3Add(player.pos, player.front);
    camera.up = player.up;
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void SpawnEnemy(Vector3 pos) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].pos = pos;
            enemies[i].health = 50.0f + wave * 10.0f;
            enemies[i].maxHealth = enemies[i].health;
            enemies[i].active = true;
            enemies[i].target = player.pos;
            enemies[i].speed = 1.0f + wave * 0.1f;
            enemies[i].lastShot = 0;
            enemies[i].color = (Color){255, 100, 100, 255};
            break;
        }
    }
}

void SpawnWave() {
    int enemiesToSpawn = 3 + wave * 2;
    
    for (int i = 0; i < enemiesToSpawn; i++) {
        Vector3 spawnPos;
        bool validPos = false;
        int attempts = 0;
        
        while (!validPos && attempts < 100) {
            spawnPos.x = GetRandomValue(5, MAP_SIZE - 5) + 0.5f;
            spawnPos.z = GetRandomValue(5, MAP_SIZE - 5) + 0.5f;
            spawnPos.y = 1.0f;
            
            int mapX = (int)spawnPos.x;
            int mapZ = (int)spawnPos.z;
            
            if (map[mapX][mapZ] == 0 && Vector3Distance(spawnPos, player.pos) > 10.0f) {
                validPos = true;
            }
            attempts++;
        }
        
        if (validPos) {
            SpawnEnemy(spawnPos);
        }
    }
}

void CreateParticle(Vector3 pos, Vector3 vel, Color color, float life) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) {
            particles[i].pos = pos;
            particles[i].vel = vel;
            particles[i].color = color;
            particles[i].life = life;
            break;
        }
    }
}

void FireBullet() {
    if (GetTime() - player.lastShot < 0.1f || player.ammo <= 0) return;
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].pos = Vector3Add(player.pos, Vector3Scale(player.front, 0.5f));
            bullets[i].vel = Vector3Scale(player.front, 50.0f);
            bullets[i].active = true;
            bullets[i].damage = 25.0f;
            player.lastShot = GetTime();
            player.ammo--;
            
            // Muzzle flash particles
            for (int j = 0; j < 5; j++) {
                Vector3 particleVel = {
                    GetRandomValue(-5, 5),
                    GetRandomValue(-2, 2),
                    GetRandomValue(-5, 5)
                };
                CreateParticle(bullets[i].pos, particleVel, YELLOW, 0.2f);
            }
            break;
        }
    }
}

bool CheckWallCollision(Vector3 pos) {
    int mapX = (int)pos.x;
    int mapZ = (int)pos.z;
    
    if (mapX < 0 || mapX >= MAP_SIZE || mapZ < 0 || mapZ >= MAP_SIZE) {
        return true;
    }
    
    return map[mapX][mapZ] == 1;
}

void UpdatePlayer() {
    Vector2 mouseDelta = GetMouseDelta();
    
    player.yaw += mouseDelta.x * 0.1f;
    player.pitch -= mouseDelta.y * 0.1f;
    
    if (player.pitch > 89.0f) player.pitch = 89.0f;
    if (player.pitch < -89.0f) player.pitch = -89.0f;
    
    // Update direction vectors
    player.front.x = cos(DEG2RAD * player.yaw) * cos(DEG2RAD * player.pitch);
    player.front.y = sin(DEG2RAD * player.pitch);
    player.front.z = sin(DEG2RAD * player.yaw) * cos(DEG2RAD * player.pitch);
    player.front = Vector3Normalize(player.front);
    player.right = Vector3Normalize(Vector3CrossProduct(player.front, (Vector3){0, 1, 0}));
    player.up = Vector3CrossProduct(player.right, player.front);
    
    // Movement
    Vector3 newPos = player.pos;
    float speed = 5.0f * GetFrameTime();
    
    if (IsKeyDown(KEY_W)) newPos = Vector3Add(newPos, Vector3Scale(player.front, speed));
    if (IsKeyDown(KEY_S)) newPos = Vector3Subtract(newPos, Vector3Scale(player.front, speed));
    if (IsKeyDown(KEY_A)) newPos = Vector3Subtract(newPos, Vector3Scale(player.right, speed));
    if (IsKeyDown(KEY_D)) newPos = Vector3Add(newPos, Vector3Scale(player.right, speed));
    
    // Check collision before moving
    if (!CheckWallCollision(newPos)) {
        player.pos = newPos;
    }
    
    // Shooting
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        FireBullet();
    }
    
    // Reload
    if (IsKeyPressed(KEY_R)) {
        player.ammo = 30;
    }
}

void UpdateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        // Move towards player
        Vector3 direction = Vector3Subtract(player.pos, enemies[i].pos);
        float distance = Vector3Length(direction);
        
        if (distance > 0.1f) {
            direction = Vector3Normalize(direction);
            Vector3 newPos = Vector3Add(enemies[i].pos, Vector3Scale(direction, enemies[i].speed * GetFrameTime()));
            
            if (!CheckWallCollision(newPos)) {
                enemies[i].pos = newPos;
            }
        }
        
        // Attack player if close enough
        if (distance < 2.0f && GetTime() - enemies[i].lastShot > 1.0f) {
            player.health -= 10.0f;
            enemies[i].lastShot = GetTime();
            
            // Blood particles
            for (int j = 0; j < 3; j++) {
                Vector3 particleVel = {
                    GetRandomValue(-3, 3),
                    GetRandomValue(1, 5),
                    GetRandomValue(-3, 3)
                };
                CreateParticle(player.pos, particleVel, RED, 0.5f);
            }
        }
    }
}

void UpdateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        bullets[i].pos = Vector3Add(bullets[i].pos, Vector3Scale(bullets[i].vel, GetFrameTime()));
        
        // Check wall collision
        if (CheckWallCollision(bullets[i].pos)) {
            bullets[i].active = false;
            
            // Spark particles
            for (int j = 0; j < 3; j++) {
                Vector3 particleVel = {
                    GetRandomValue(-2, 2),
                    GetRandomValue(-2, 2),
                    GetRandomValue(-2, 2)
                };
                CreateParticle(bullets[i].pos, particleVel, ORANGE, 0.3f);
            }
            continue;
        }
        
        // Check enemy collision
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            
            if (Vector3Distance(bullets[i].pos, enemies[j].pos) < 0.5f) {
                enemies[j].health -= bullets[i].damage;
                bullets[i].active = false;
                
                // Hit particles
                for (int k = 0; k < 5; k++) {
                    Vector3 particleVel = {
                        GetRandomValue(-3, 3),
                        GetRandomValue(1, 4),
                        GetRandomValue(-3, 3)
                    };
                    CreateParticle(enemies[j].pos, particleVel, RED, 0.4f);
                }
                
                if (enemies[j].health <= 0) {
                    enemies[j].active = false;
                    score += 100;
                    enemiesKilled++;
                    
                    // Death explosion
                    for (int k = 0; k < 10; k++) {
                        Vector3 particleVel = {
                            GetRandomValue(-5, 5),
                            GetRandomValue(2, 8),
                            GetRandomValue(-5, 5)
                        };
                        CreateParticle(enemies[j].pos, particleVel, RED, 0.8f);
                    }
                }
                break;
            }
        }
        
        // Remove bullets that are too far
        if (Vector3Distance(bullets[i].pos, player.pos) > 100.0f) {
            bullets[i].active = false;
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) continue;
        
        particles[i].life -= GetFrameTime();
        particles[i].pos = Vector3Add(particles[i].pos, Vector3Scale(particles[i].vel, GetFrameTime()));
        particles[i].vel.y -= 9.8f * GetFrameTime(); // Gravity
    }
}

void DrawMap() {
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (map[i][j] == 1) {
                DrawCube((Vector3){i + 0.5f, 1.0f, j + 0.5f}, 1.0f, 2.0f, 1.0f, GRAY);
                DrawCubeWires((Vector3){i + 0.5f, 1.0f, j + 0.5f}, 1.0f, 2.0f, 1.0f, DARKGRAY);
            }
        }
    }
    
    // Floor
    DrawPlane((Vector3){MAP_SIZE/2, 0, MAP_SIZE/2}, (Vector2){MAP_SIZE, MAP_SIZE}, LIGHTGRAY);
}

void DrawEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        DrawCube(enemies[i].pos, 0.8f, 1.8f, 0.8f, enemies[i].color);
        DrawCubeWires(enemies[i].pos, 0.8f, 1.8f, 0.8f, RED);
        
        // Health bar
        Vector3 barPos = Vector3Add(enemies[i].pos, (Vector3){0, 1.2f, 0});
        float healthPercent = enemies[i].health / enemies[i].maxHealth;
        DrawCube(Vector3Add(barPos, (Vector3){-0.2f, 0, 0}), 0.4f * healthPercent, 0.1f, 0.05f, GREEN);
        DrawCube(Vector3Add(barPos, (Vector3){0.2f - 0.2f * healthPercent, 0, 0}), 0.4f * (1 - healthPercent), 0.1f, 0.05f, RED);
    }
}

void DrawBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        DrawSphere(bullets[i].pos, 0.05f, YELLOW);
    }
}

void DrawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) continue;
        
        Color color = particles[i].color;
        color.a = (unsigned char)(255 * (particles[i].life / 1.0f));
        DrawSphere(particles[i].pos, 0.02f, color);
    }
}

void DrawHUD() {
    // Health bar
    DrawRectangle(10, 10, 200, 20, RED);
    DrawRectangle(10, 10, (int)(200 * (player.health / player.maxHealth)), 20, GREEN);
    DrawText("HEALTH", 15, 12, 16, WHITE);
    
    // Ammo
    DrawText(TextFormat("AMMO: %d", player.ammo), 10, 40, 20, WHITE);
    
    // Score and wave
    DrawText(TextFormat("SCORE: %d", score), 10, 70, 20, WHITE);
    DrawText(TextFormat("WAVE: %d", wave), 10, 100, 20, WHITE);
    DrawText(TextFormat("ENEMIES: %d", enemiesKilled), 10, 130, 20, WHITE);
    
    // Crosshair
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
    DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
    
    // Game over screen
    if (gameOver) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f));
        DrawText("GAME OVER", GetScreenWidth()/2 - 100, GetScreenHeight()/2 - 50, 40, RED);
        DrawText(TextFormat("Final Score: %d", score), GetScreenWidth()/2 - 80, GetScreenHeight()/2, 20, WHITE);
        DrawText("Press R to restart", GetScreenWidth()/2 - 70, GetScreenHeight()/2 + 30, 20, WHITE);
    }
}