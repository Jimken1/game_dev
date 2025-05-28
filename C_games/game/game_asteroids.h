#ifndef ASTEROIDS_H_
#define ASTEROIDS_H_

#include "asteroid.h"

void AddAsteroid(Vector2 position, AsteroidSize size);
Vector2 GetAsteroidNextPosition(void);
int UpdateAsteroids(void);
void DrawAsteroids(void);

#endif