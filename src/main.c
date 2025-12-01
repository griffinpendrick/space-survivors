#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "common.h"
#include "particle.c"
#include "player.c"
#include "projectile.c"
#include "enemy.c"

global Texture2D EnemyTexture;
global Texture2D PlayerTexture;
global Sound ExplosionSound;
global Sound ThrusterSound;
global Sound ButtonClickSound;

#include "ui.c"

#define BACKGROUND_COLOR (Color){15, 15, 15, 255}
#define MAX_STARS 128
global Vector2 Stars[MAX_STARS];

typedef enum state_type state_type;
enum state_type
{
	MainMenu,
	Controls,
	Playing,
	Paused,
	GameOver,
	Quit,
};

typedef struct game_state game_state;
struct game_state
{
    state_type Type;
    state_type LastType;

    player Player;
    enemy_pool Enemies;
    particle_system Particles;
    projectile_pool Projectiles;
    Camera2D Camera;
    f32 CameraShakeStrength;
    bool32 IsRunning;
};

internal void InitGame(game_state* State)
{
    State->Particles = (particle_system){0};
    State->Projectiles = (projectile_pool){0};

    State->Enemies = (enemy_pool){0};
    State->Enemies.CurrentWave = 1;
    State->Enemies.EnemiesToSpawn = 10;
    State->Enemies.EnemiesRemaining = 10;
    State->Enemies.Speed = 50.0f;
    State->Enemies.Damage = 1.0f;

    State->Player = (player){0};
    State->Player.Position = WindowCenter;
    State->Player.Acceleration = (Vector2){300.0f, 300.0f};
    State->Player.Health = 100.0f;

    State->Camera = (Camera2D){0};
    State->Camera.target = WindowCenter;
    State->Camera.offset = WindowCenter;
    State->Camera.zoom = 1.0f;
    State->CameraShakeStrength = 0.0f;

    SpawnEnemies(&State->Enemies, State->Player.Position, State->Enemies.EnemiesToSpawn);
}

INLINE void ShakeCamera(game_state* State)
{
    State->CameraShakeStrength = 10.0f;
}

internal void UpdateGameCamera(game_state* State, f32 dt)
{
    State->CameraShakeStrength = Lerp(State->CameraShakeStrength, 0.0f, 3.0f * dt);
    State->Camera.offset.x = (WindowWidth / 2.0f) + (RandomFloat(-1.0f, 1.0f) * State->CameraShakeStrength);
    State->Camera.offset.y = (WindowHeight / 2.0f) + (RandomFloat(-1.0f, 1.0f) * State->CameraShakeStrength);
}

internal void ResolveCollisions(game_state* State)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(State->Projectiles.Active[i])
        {
            for(int32 j = 0; j < MAX_ENEMIES; j++)
            {
                if(State->Enemies.Active[j])
                {
                    if(Vector2Distance(State->Projectiles.Positions[i], State->Enemies.Positions[j]) < 32.0f)
                    {
                        State->Enemies.Active[j] = false;
                        State->Enemies.EnemiesRemaining--;
                        State->Projectiles.Active[i] = false;

                        EmitParticles(&State->Particles, State->Enemies.Positions[j], RED, 32);
                        PlaySound(ExplosionSound);
                        ShakeCamera(State);
                        break;
                    }
                }
            }
        }
    }
    for(int32 i = 0; i < MAX_ENEMIES; i++)
    {
        if(State->Enemies.Active[i])
        {
            if(Vector2Distance(State->Player.Position, State->Enemies.Positions[i]) < 32.0f)
            {
                if(State->Player.DamageCooldown <= 0.0f)
                {
                    State->Player.Health -= State->Enemies.Damage;
                    State->Player.DamageCooldown = 0.25f;
                }
                if(State->Player.Health <= 0)
                {
                    State->Type = GameOver;
                }
                break;
            }
        }
    }
}

INLINE void UpdateBackground(game_state* State, f32 dt)
{
    for(int32 i = 0; i < MAX_STARS; i++)
	{
		Stars[i] = Vector2Subtract(Stars[i], Vector2Scale(State->Player.Velocity, 0.25f * dt));

		Stars[i].x = Wrap(Stars[i].x, 0.0f, (f32)WindowWidth);
		Stars[i].y = Wrap(Stars[i].y, 0.0f, (f32)WindowHeight);
	}
}

internal void DrawBackground(void)
{
	ClearBackground(BACKGROUND_COLOR);

    rlSetTexture(0);
    rlBegin(RL_QUADS);
    rlColor4ub(130, 130, 130, 255);

    for(int32 i = 0; i < MAX_STARS; i++)
    {
        f32 X = Stars[i].x;
        f32 Y = Stars[i].y;

        rlVertex2f(X - 1.0f, Y - 1.0f);
        rlVertex2f(X + 1.0f, Y - 1.0f);
        rlVertex2f(X + 1.0f, Y + 1.0f);
        rlVertex2f(X - 1.0f, Y + 1.0f);
    }

    rlEnd();
}

internal void DrawMainMenu(game_state* State)
{
	DrawText("Space Survivors", (WindowWidth / 2) - (MeasureText("Space Survivors", 64) / 2), (WindowHeight / 2) - 200, 64, WHITE);
	if(DoButton("Play", (WindowWidth / 2) - (MeasureText("Play", 48) / 2), (WindowHeight / 2), 48))
	{
	    State->Type = Playing;
		InitGame(State);
	}
	if(DoButton("Controls", (WindowWidth / 2) - (MeasureText("Controls", 48) / 2), (WindowHeight / 2) + 50, 48))
	{
	    State->LastType = MainMenu;
	    State->Type = Controls;
	}
	if(DoButton("Quit", (WindowWidth / 2) - (MeasureText("Quit", 48) / 2), (WindowHeight / 2) + 100, 48))
	{
	    State->Type = Quit;
	}
}

internal void DrawPausedMenu(game_state* State)
{
	DrawText("Paused", (WindowWidth / 2) - (MeasureText("Paused", 48) / 2), (WindowHeight / 2) - 200, 48, WHITE);
	if(DoButton("Resume", (WindowWidth / 2) - (MeasureText("Resume", 48) / 2), (WindowHeight / 2), 48))
	{
	    State->Type = Playing;
	}
	if(DoButton("Controls", (WindowWidth / 2) - (MeasureText("Controls", 48) / 2), (WindowHeight / 2) + 50, 48))
	{
	    State->LastType = Paused;
	    State->Type = Controls;
	}
	if(DoButton("Main Menu", (WindowWidth / 2) - (MeasureText("Main Menu", 48) / 2), (WindowHeight / 2) + 100, 48))
	{
	    State->Type = MainMenu;
	}
}

internal void DrawControlsMenu(game_state* State)
{
    DrawText("Controls", (WindowWidth / 2) - (MeasureText("Controls", 48) / 2), (WindowHeight / 2) - 220, 48, WHITE);
    DrawText("W - Move Forward", (WindowWidth / 2) - (MeasureText("W - Move Forward", 48) / 2), (WindowHeight / 2) - 100, 48, WHITE);
    DrawText("A - Rotate Left", (WindowWidth / 2) - (MeasureText("A - Rotate Left", 48) / 2), (WindowHeight / 2) - 50, 48, WHITE);
    DrawText("D - Rotate Right", (WindowWidth / 2) - (MeasureText("D - Rotate Right", 48) / 2), (WindowHeight / 2), 48, WHITE);
    DrawText("SPACE - Shoot", (WindowWidth / 2) - (MeasureText("SPACE - Shoot", 48) / 2), (WindowHeight / 2) + 50, 48, WHITE);
    DrawText("ESC - Pause / Back", (WindowWidth / 2) - (MeasureText("ESC - Pause / Back", 48) / 2), (WindowHeight / 2) + 100, 48, WHITE);
    if(DoButton("Back", (WindowWidth / 2) - (MeasureText("Back", 48) / 2), WindowHeight - 100, 48))
    {
        State->Type = State->LastType;
    }
}


internal void DrawGameOver(game_state* State)
{
	DrawText("Game Over!", (WindowWidth / 2) - (MeasureText("Game Over!", 64) / 2), (WindowHeight / 2) - 200, 64, WHITE);
	if(DoButton("Restart", (WindowWidth / 2) - (MeasureText("Restart", 48) / 2), (WindowHeight / 2), 48))
	{
	    State->Type = Playing;
		InitGame(State);
	}
	if(DoButton("Back to Main Menu", (WindowWidth / 2) - (MeasureText("Back to Main Menu", 48) / 2), (WindowHeight / 2) + 50, 48))
	{
	    State->Type = MainMenu;
	}
}

// NOTE(griffin): TEMP
internal void DrawHUD(game_state* State)
{
    const f32 MaxHealth = 100.0f;
    f32 Health = Clamp(State->Player.Health, 0.0f, MaxHealth);

    f32 BarWidth  = 200.0f;
    f32 BarHeight = 20.0f;

    f32 X = 20.0f;
    f32 Y = WindowHeight - BarHeight - 20.0f;

    f32 HealthPercent = Health / MaxHealth;
    f32 FillWidth = BarWidth * HealthPercent;

    DrawRectangle((int32)X, (int32)Y, (int32)BarWidth, (int32)BarHeight, BLACK);
    DrawRectangle((int32)X, (int32)Y, (int32)FillWidth, (int32)BarHeight, RED);
    DrawRectangleLines((int32)X, (int32)Y, (int32)BarWidth, (int32)BarHeight, WHITE);

    char HealthBarLabel[64];
    snprintf(HealthBarLabel, sizeof(HealthBarLabel), "Health: %.0f", Health);
    DrawText(HealthBarLabel, (int32)(X + (BarWidth / 2) - (MeasureText(HealthBarLabel, 18) / 2)), (int32)(Y + (BarHeight / 2) - 9), 18, WHITE);

    char WaveText[64];
    snprintf(WaveText, sizeof(WaveText), "Wave: %d", State->Enemies.CurrentWave);
    DrawText(WaveText, WindowWidth - MeasureText(WaveText, 32) - 20, 20, 32, WHITE);
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI /*| FLAG_VSYNC_HINT*/);
    InitWindow(WindowWidth, WindowHeight, "Space Survivors");
    InitAudioDevice();

	SetExitKey(KEY_NULL);

    rlDisableBackfaceCulling();
    rlDisableDepthTest();

    EnemyTexture = LoadTexture("../assets/enemy.png");
    PlayerTexture = LoadTexture("../assets/ship.png");
    ExplosionSound = LoadSound("../assets/sfx/explosion.mp3");
    ThrusterSound = LoadSound("../assets/sfx/thruster.mp3");
    ButtonClickSound = LoadSound("../assets/sfx/button.mp3");

    for(int32 i = 0; i < MAX_STARS; i++)
    {
        Stars[i] = (Vector2){RandomFloat(-250.0f, (f32)WindowWidth + 250.0f), RandomFloat(-250.0f, (f32)WindowHeight) + 250.0f};
    }

    game_state State = {0};
    State.IsRunning = true;

    while(!WindowShouldClose() && State.IsRunning)
    {
        f32 dt = GetFrameTime();

		BeginDrawing();
		DrawBackground();
		switch(State.Type)
		{
			case MainMenu:
			{
				DrawMainMenu(&State);
				break;
			}
			case Controls:
			{
			    if(IsKeyPressed(KEY_ESCAPE))
				{
				    State.Type = State.LastType;
				}

				DrawControlsMenu(&State);
				break;
			}
			case Playing:
			{
					if(IsKeyDown(KEY_W))
					{
						AcceleratePlayer(&State.Player, dt);

						State.Particles.Accumulator += dt;
						if(State.Particles.Accumulator >= 0.05f)
						{
							f32 Radians = State.Player.Angle * DEG2RAD;
							Vector2 Direction = (Vector2){sinf(Radians), -cosf(Radians)};
							Vector2 FirePosition = Vector2Subtract(State.Player.Position, Vector2Scale(Direction, PlayerTexture.height * 0.5f));
							EmitParticles(&State.Particles, FirePosition, ORANGE, 1);
							State.Particles.Accumulator = 0.0f;
						}
						if(!IsSoundPlaying(ThrusterSound))
						{
						    PlaySound(ThrusterSound);
						}
					}
					else
					{
					    if(IsSoundPlaying(ThrusterSound))
					    {
							StopSound(ThrusterSound);
						}
					}
					if(IsKeyDown(KEY_A))
					{
						State.Player.Angle -= ROTATION_SPEED * dt;
					}
					if(IsKeyDown(KEY_D))
					{
						State.Player.Angle += ROTATION_SPEED * dt;
					}
					if(IsKeyPressed(KEY_SPACE))
					{
						SpawnProjectile(&State.Projectiles, State.Player.Position, State.Player.Angle * DEG2RAD);
					}
					if(IsKeyPressed(KEY_ESCAPE))
					{
					    State.Type = Paused;
					}

					UpdateParticles(&State.Particles, dt);
					UpdateProjectiles(&State.Projectiles, dt);
					UpdateEnemies(&State.Enemies, State.Player.Position, dt);
					UpdatePlayer(&State.Player, dt);
					UpdateGameCamera(&State, dt);
					UpdateBackground(&State, dt);

					ResolveCollisions(&State);
					if(State.Enemies.EnemiesRemaining <= 0)
					{
					    StartNextWave(&State.Enemies, State.Player.Position);
					}

					BeginMode2D(State.Camera);

					DrawPlayer(State.Player, PlayerTexture);
					DrawProjectiles(State.Projectiles);
					DrawEnemies(State.Enemies, EnemyTexture);
					DrawParticles(&State.Particles);

					EndMode2D();
					DrawHUD(&State);
					break;
				}
				case Paused:
				{
				    if(IsKeyPressed(KEY_ESCAPE))
					{
					    State.Type = Playing;
					}

					BeginMode2D(State.Camera);

					DrawPlayer(State.Player, PlayerTexture);
					DrawProjectiles(State.Projectiles);
					DrawEnemies(State.Enemies, EnemyTexture);
					DrawParticles(&State.Particles);

					EndMode2D();
					DrawHUD(&State);
					DrawPausedMenu(&State);
					break;
				}
				case GameOver:
		        {
					if(IsSoundPlaying(ThrusterSound))
					{
					    StopSound(ThrusterSound);
					}

					DrawGameOver(&State);
					break;
				}
				case Quit:
				{
					State.IsRunning = false;
					break;
				}
			}
			DrawFPS(10, 10);
			EndDrawing();
		}

    CloseWindow();
    return(0);
}
