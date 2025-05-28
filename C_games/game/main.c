#include <stdlib.h>

#include <time.h>
#include "raylib.h"
#include "raymath.h"
#include "raygui.c"
#include "debug.h"
#include "debug.c"
#include "constants.h"
#include "asteroid.c"
#include "game_asteroids.c"
 





 


void UpdateDrawFrame(void);

int main()
{
    srand(time(0));
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "asteroids");
    
    while(!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
    
    CloseWindow();
    
    return 0;
    
}


void UpdateDrawFrame(void)
{
    
   
   int activeAsteroids = UpdateAsteroids();
    BeginDrawing();
       ClearBackground(NEARBLACK);
       
      DrawAsteroids();

    
   
    showDebugVisualisations(activeAsteroids);
    showDebugMenu();
    EndDrawing();
}








