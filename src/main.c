#include <stdlib.h>
#include <time.h>

#pragma warning(push, 0)
#include "raylib.h"
#include "raymath.h"
#pragma warning pop

#include "utils.h"
#include "particle.c"
#include "player.c"
#include "projectile.c"
#include "enemy.c"

#define BACKGROUND_COLOR (Color){15, 15, 15, 255}
#define MAX_STARS 128
#define PARALLAX_STRENGTH 0.25f

global int32 WindowWidth = 1280;
global int32 WindowHeight = 720;

global_const f32 ShakeDecayRate = 3.0f;
global f32 CameraShakeStrength = 0.0f;

internal void UpdateCamera(Camera2D* Camera, f32 dt)
{
    CameraShakeStrength = Lerp(CameraShakeStrength, 0, ShakeDecayRate * dt);
    Camera->offset.x = WindowWidth / 2.0f + RandomFloat(-1.0f, 1.0f) * CameraShakeStrength;
    Camera->offset.y = WindowHeight / 2.0f + RandomFloat(-1.0f, 1.0f) * CameraShakeStrength;
}

internal void ResolveCollisions(projectile_pool* Projectiles, enemy_pool* Enemies, particle_system* Particles)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(Projectiles->Active[i])
        {
            for(int32 j = 0; j < MAX_ENEMIES; j++)
            {
                if(Enemies->Active[j])
                {
                    // FIXME(griffin): Guesstimate for the collision radius, probably should fix later.
                    if(Vector2Distance(Projectiles->Position[i], Enemies->Position[j]) < 16.0f)
                    {
                        EmitParticles(Particles, Enemies->Position[j], RED, 32);
                        Projectiles->Active[i] = false;
                        Enemies->Active[j] = false;
                        CameraShakeStrength = 10.0f;
                        break;
                    }
                }
            }
        }
    }
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(WindowWidth, WindowHeight, "Space Survivors");
    SetTargetFPS(60);

    Image WindowIcon = LoadImage("..\\assets\\enemy.png");
    SetWindowIcon(WindowIcon);

    srand((uint32)time(NULL));

    Vector2 Stars[MAX_STARS];
    for(int32 i = 0; i < MAX_STARS; i++)
    {
        Stars[i] = (Vector2){RandomFloat(0.0f, WindowWidth), RandomFloat(0.0f, WindowHeight)};
    }

    particle_system Particles;
    memset(&Particles, 0, sizeof(particle_system));

    projectile_pool Projectiles;
    memset(&Projectiles, 0, sizeof(projectile_pool));

    enemy_pool Enemies;
    memset(&Enemies, 0, sizeof(enemy_pool));
    Enemies.Texture = LoadTexture("..\\assets\\enemy.png");

    player Player;
    Player.Texture = LoadTexture("..\\assets\\ship.png");
    Player.Position = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};
    Player.Velocity = (Vector2){0.0f, 0.0f};
    Player.Acceleration = (Vector2){300.0f, 300.0f};
    Player.Angle = 0.0f;

    Camera2D Camera = {0};
    Camera.target = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};
    Camera.offset = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};
    Camera.rotation = 0.0f;
    Camera.zoom = 1.0f;

    // NOTE(griffin): TEMP
    SpawnEnemies(&Enemies, Player.Position, 10);

    while(!WindowShouldClose())
    {
        f32 dt = GetFrameTime();
        int32 NewWindowWidth = GetScreenWidth();
        int32 NewWindowHeight = GetScreenHeight();

        Camera.zoom = 1.0f;
        Camera.target = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};

        if(NewWindowWidth != WindowWidth || NewWindowHeight != WindowHeight)
        {
            WindowWidth = NewWindowWidth;
            WindowHeight = NewWindowHeight;
            Camera.target = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};
            Camera.offset = (Vector2){WindowWidth / 2.0f, WindowHeight / 2.0f};

            for(int32 i = 0; i < MAX_STARS; i++)
            {
                Stars[i] = (Vector2){RandomFloat(0.0f, WindowWidth), RandomFloat(0.0f, WindowHeight)};
            }
        }

        UpdateParticles(&Particles, dt);
        UpdateProjectiles(&Projectiles, WindowWidth, WindowHeight, dt);
        UpdateEnemies(&Enemies, Player.Position, dt);
        UpdatePlayer(&Player, WindowWidth, WindowHeight, dt);
        UpdateCamera(&Camera, dt);

        for(int32 i = 0; i < MAX_STARS; i++)
        {
            Stars[i].x -= Player.Velocity.x * PARALLAX_STRENGTH * dt;
            Stars[i].y -= Player.Velocity.y * PARALLAX_STRENGTH * dt;

            if(Stars[i].x < 0.0f)
            {
                Stars[i].x += WindowWidth;
            }
            if(Stars[i].x > WindowWidth)
            {
                Stars[i].x -= WindowWidth;
            }
            if(Stars[i].y < 0.0f)
            {
                Stars[i].y += WindowHeight;
            }
            if(Stars[i].y > WindowHeight)
            {
                Stars[i].y -= WindowHeight;
            }
        }

        ResolveCollisions(&Projectiles, &Enemies, &Particles);

        if(IsKeyDown(KEY_W))
        {
            AcceleratePlayer(&Player, dt);

            f32 Radians = Player.Angle * DEG2RAD;
            Vector2 Direction = (Vector2){sinf(Radians), -cosf(Radians)};
            Vector2 FirePosition = Vector2Subtract(Player.Position, Vector2Scale(Direction, Player.Texture.height * 0.5f));
            EmitParticles(&Particles, FirePosition, ORANGE, 1);
        }
        if(IsKeyDown(KEY_A))
        {
            Player.Angle -= ROTATION_SPEED * dt;
        }
        if(IsKeyDown(KEY_D))
        {
            Player.Angle += ROTATION_SPEED * dt;
        }
        if(IsKeyPressed(KEY_SPACE))
        {
            SpawnProjectile(&Projectiles, Player.Position, Player.Angle * DEG2RAD);
        }
        if(IsKeyDown(KEY_E))
        {
            Camera.zoom = 2.0f;
            Camera.target = Player.Position;
        }

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        BeginMode2D(Camera);

        for(int32 i = 0; i < MAX_STARS; i++)
        {
            DrawCircleV(Stars[i], 1.0f, GRAY);
        }

        DrawPlayer(Player);
        DrawProjectiles(Projectiles);
        DrawEnemies(Enemies);
        DrawParticles(&Particles);

        EndMode2D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadImage(WindowIcon);
    UnloadTexture(Player.Texture);
    UnloadTexture(Enemies.Texture);

    CloseWindow();
    return(0);
}