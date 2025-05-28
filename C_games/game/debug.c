#include "debug.h"


#include "constants.h"
#include "raymath.h"
// #include "raygui.c"


static bool _showDebugMenu = false;
static bool _debugUseCheckbox = false;
static bool _showAsteroidCount = false;
static bool _showAngleCone = false;
static Vector2 line0[2] = {0};

static Vector2 line1[2] = {0};

void showDebugMenu(void)
{
    if (IsKeyPressed(KEY_F6))
    {
        _showDebugMenu = !_showDebugMenu;
    }
    
    if (_showDebugMenu)
    {
      
        
        if(!_debugUseCheckbox)
        {
            Rectangle r = {10, SCREEN_HEIGHT - 80, 160, 60};
            GuiToggle(r, "show asteroid count", &_showAsteroidCount);
            r.x +=160;
            GuiToggle(r, "show angle cones", &_showAngleCone);
            r.x +=160;
            if (GuiToggle(r, "switch menu style", &_debugUseCheckbox))
                    {
                        _debugUseCheckbox = !_debugUseCheckbox;
                    }
            
        }

        else 
        {
            Rectangle r = {10, SCREEN_HEIGHT - 40, 20, 20};
           GuiCheckBox(r, "Show Asteroid count", &_showAsteroidCount);
            r.y -= 30;
            GuiCheckBox(r, "Show Angle Cone", &_showAngleCone);
            r.y -= 30;
            if (GuiCheckBox(r, "Switch Menu Style", false))
            {
                _debugUseCheckbox = !_debugUseCheckbox;
            }

        }
         
         
     

    }

}

void showDebugVisualisations(int asteroidCount)
{
    if(_showAngleCone){
            DrawLineV(line0[0], line0[1], RED);
            DrawLineV(line1[0], line1[1], BLUE);
        }
        
          
    if (_showAsteroidCount)
    {
       
        DrawRectangle(10, 10, 100, 52, Fade(BLACK, 0.6f));
        DrawText(TextFormat("ASTEROIDS: %d", asteroidCount), 20, 20, 32, WHITE);
    }
}

void setLastCone(Vector2 position, Vector2 velocity)
{

        line0[0] = position;
        line1[0] = position;
        
        line0[1] = Vector2Add(position, Vector2Rotate(Vector2Scale(velocity, 10), -ASTEROID_RANDOM_ANGLE));
        line1[1] = Vector2Add(position, Vector2Rotate(Vector2Scale(velocity, 10), ASTEROID_RANDOM_ANGLE));
    
  
}