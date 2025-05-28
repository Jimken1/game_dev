#include "game_asteroids.h"
#include "constants.h"
#include "debug.h"
static AsteroidSize _sizes[] = {ASTEROID_SMALL, ASTEROID_MEDIUM, ASTEROID_LARGE};
static Asteroid _asteroids[ASTEROID_MAX] = {0};
static float _lastAsteroidCreationTime = -1.0f;

void AddAsteroid(Vector2 position, AsteroidSize size)
{
    bool created = false;
    Vector2 velocity = Vector2Subtract(SCREEN_CENTER, position);
    velocity = Vector2Scale(Vector2Normalize(velocity), GetRandomValue(ASTEROID_SPEED_MIN, ASTEROID_SPEED_MAX));
   
   setLastCone(position, velocity);

    velocity = Vector2Rotate(velocity, (float)GetRandomValue(ASTEROID_RANDOM_ANGLE, ASTEROID_RANDOM_ANGLE));
       
    for (int i = 0; i<ASTEROID_MAX; i++)
    {
        if(_asteroids[i].active)
        {
            continue;
        }
        
        _asteroids[i] = CreateAsteroid(position, velocity, size);
        created = true;
        break;
    }
    
    if (!created)
    {
        TraceLog(LOG_ERROR, "Failed to create asteroid because there was no inactive spost on the array!");
    }
    
}

Vector2 GetAsteroidNextPosition(void)
{
    int padding = 128;
    Vector2 result = {-padding, -padding};
    
    if (GetRandomValue(0, 1))
    {
        if (GetRandomValue(0, 1))
        {
            result.y = SCREEN_WIDTH + padding;
        }
        
        result.x = GetRandomValue(-padding, SCREEN_WIDTH + padding);
        
    }
    else 
    {
        if (GetRandomValue(0, 1))
        {
            result.x = SCREEN_WIDTH + padding;
        }
        
        result.y = GetRandomValue(-padding, SCREEN_HEIGHT + padding);
    }
    
    return result;    
    
}

int UpdateAsteroids(void)
{
    int activeAsteroids = 0;

  float frametime = GetFrameTime();
    float time = GetTime();
    
    
    for (int i = 0; i<ASTEROID_MAX;i++)
    {
           
        if (AsteroidUpdate(_asteroids + i, frametime, time))
        {
            activeAsteroids++;
        }
    }
    
    if (time > _lastAsteroidCreationTime + ASTEROID_DELAY)
    {        
        AsteroidSize nextSize =  _sizes[GetRandomValue(0, 3)];
        AddAsteroid(GetAsteroidNextPosition(), nextSize);
        _lastAsteroidCreationTime = time;
    }

    return activeAsteroids;
}

void DrawAsteroids(void)
{
    for (int i = 0; i < ASTEROID_MAX; i++)
    {
        AsteroidDraw(_asteroids[i]);
    }
}