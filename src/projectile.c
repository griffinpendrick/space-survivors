#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#define MAX_PROJECTILES 32
#define PROJECTILE_SPEED 400.0f

typedef struct
{
    Vector2 Position[MAX_PROJECTILES];
    f32 Angle[MAX_PROJECTILES];
    bool32 Active[MAX_PROJECTILES];
} projectile_pool;

void SpawnProjectile(projectile_pool* Projectiles, Vector2 Position, f32 Angle)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(!Projectiles->Active[i])
        {
            Projectiles->Position[i] = Position;
            Projectiles->Angle[i] = Angle;
            Projectiles->Active[i] = true;
            break;
        }
    }
}

void UpdateProjectiles(projectile_pool* Projectiles, int32 WindowWidth, int32 WindowHeight, f32 dt)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(Projectiles->Active[i])
        {
            if(Projectiles->Position[i].x < 0 || Projectiles->Position[i].x > WindowWidth ||
               Projectiles->Position[i].y < 0 || Projectiles->Position[i].y > WindowHeight)
            {
                Projectiles->Active[i] = false;
            }
            else
            {
                Vector2 Direction = (Vector2){sinf(Projectiles->Angle[i]), -cosf(Projectiles->Angle[i])};
                Projectiles->Position[i] = Vector2Add(Projectiles->Position[i], Vector2Scale(Direction, PROJECTILE_SPEED * dt));
            }
        }
    }
}

internal inline void DrawProjectiles(const projectile_pool Projectiles)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(Projectiles.Active[i])
        {
            Vector2 Position = Projectiles.Position[i];
            Vector2 Direction = (Vector2){sinf(Projectiles.Angle[i]), -cosf(Projectiles.Angle[i])};

            DrawCircleV(Position, 3.0f, SKYBLUE);

            for(int32 j = 0; j <= 8; j++)
            {
                Vector2 Trail = Vector2Subtract(Position, Vector2Scale(Direction, j * 3.0f));
                Color Fade = SKYBLUE;
                Fade.a = (uint8)Clamp((200 - j * 15), 0, 255);
                DrawCircleV(Trail, 2.0f, Fade);
            }
        }
    }
}
