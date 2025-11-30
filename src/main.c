#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "common.h"
#include "particle.c"
#include "player.c"
#include "projectile.c"
#include "enemy.c"

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
	Exit,
};

typedef struct game_state game_state;
struct game_state
{
    state_type Type;
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

    State->Player = (player){0};
    State->Player.Position = WindowCenter;
    State->Player.Acceleration = (Vector2){300.0f, 300.0f};

    State->Camera = (Camera2D){0};
    State->Camera.target = WindowCenter;
    State->Camera.offset = WindowCenter;
    State->Camera.zoom = 1.0f;

    // NOTE(griffin): TEMP
    SpawnEnemies(&State->Enemies, State->Player.Position, 10);
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

internal bool32 ResolveCollisions(game_state* State)
{
    for(int32 i = 0; i < MAX_PROJECTILES; i++)
    {
        if(State->Projectiles.Active[i])
        {
            for(int32 j = 0; j < MAX_ENEMIES; j++)
            {
                if(State->Enemies.Active[j])
                {
                    // FIXME(griffin): Guesstimate for the collision radius, probably should fix later.
                    if(Vector2Distance(State->Projectiles.Positions[i], State->Enemies.Positions[j]) < 32.0f)
                    {
                        State->Enemies.Active[j] = false;
                        State->Projectiles.Active[i] = false;
                        EmitParticles(&State->Particles, State->Enemies.Positions[j], RED, 32);
						return(true);
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
                State->Type = GameOver;
                return(false);
            }
        }
    }

	return(false);
}

internal void DrawBackground(void)
{
	ClearBackground(BACKGROUND_COLOR);

    rlSetTexture(0);
    rlBegin(RL_QUADS);

	Color C = GRAY;
    rlColor4ub(C.r, C.g, C.b, C.a);

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

internal void DrawHUD(void)
{
    DrawFPS(10, 10);
}

// NOTE(griffin): TEMP
internal void DrawMainMenu(void)
{
	DrawBackground();
	int FirstOffset = MeasureText("Space Survivors", 48);
	int SecondOffset = MeasureText("Play", 48);
	int ThirdOffset = MeasureText("Controls", 48);
	int FourthOffset = MeasureText("Exit", 48);
	DrawText("Space Survivors", (WindowWidth / 2) - (FirstOffset / 2), (WindowHeight / 2) - 200, 48, WHITE);
	DrawText("Play", (WindowWidth / 2) - (SecondOffset / 2), (WindowHeight / 2), 48, WHITE);
	DrawText("Controls", (WindowWidth / 2) - (ThirdOffset / 2), (WindowHeight / 2) + 50, 48, WHITE);
	DrawText("Exit", (WindowWidth / 2) - (FourthOffset / 2), (WindowHeight / 2) + 100, 48, WHITE);
}

// NOTE(griffin): TEMP
internal void DrawPausedMenu(void)
{
	int Offset = MeasureText("Paused", 48);
	DrawText("Paused", (WindowWidth / 2) - (Offset / 2), (WindowHeight / 2) - 200, 48, WHITE);
}

// NOTE(griffin): TEMP
internal void DrawControlsMenu(void)
{
	DrawBackground();
	int Offset = MeasureText("Controls", 48);
	DrawText("Controls", (WindowWidth / 2) - (Offset / 2), (WindowHeight / 2) - 200, 48, WHITE);
}

// NOTE(griffin): TEMP
internal void DrawGameOver(void)
{
    DrawBackground();
	int Offset = MeasureText("Game Over!", 48);
	DrawText("Game Over!", (WindowWidth / 2) - (Offset / 2), (WindowHeight / 2) - 200, 48, WHITE);
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI /*| FLAG_VSYNC_HINT*/);
    InitWindow(WindowWidth, WindowHeight, "Space Survivors");

    InitAudioDevice();
    SetMasterVolume(0.1f);

	SetExitKey(KEY_NULL);

    rlDisableBackfaceCulling();
    rlDisableDepthTest();

	#if defined(_WIN32)
    Image WindowIcon = LoadImage("../assets/enemy.png");
    SetWindowIcon(WindowIcon);
	#endif

    for(int32 i = 0; i < MAX_STARS; i++)
    {
        Stars[i] = (Vector2){RandomFloat(-250.0f, (f32)WindowWidth + 250.0f), RandomFloat(-250.0f, (f32)WindowHeight) + 250.0f};
    }

    Texture2D EnemyTexture = LoadTexture("../assets/enemy.png");
    Texture2D PlayerTexture = LoadTexture("../assets/ship.png");

    Sound ExplosionSound = LoadSound("../assets/sfx/explosion.mp3");
    Sound ThrusterSound = LoadSound("../assets/sfx/thruster.mp3");

    game_state State = {0};
    State.IsRunning = true;

    while(!WindowShouldClose() && State.IsRunning)
    {
        f32 dt = GetFrameTime();

		if(State.Type == MainMenu)
		{
			if(IsKeyPressed(KEY_ONE))
			{
				State.Type = Playing;
				InitGame(&State);
			}
			else if(IsKeyPressed(KEY_TWO))
			{
				State.Type = Controls;
			}
			else if(IsKeyPressed(KEY_THREE))
			{
				State.Type = Exit;
			}
		}
		if(IsKeyPressed(KEY_ESCAPE))
		{
			switch(State.Type)
			{
				case GameOver:
				case Controls:
				{
					State.Type = MainMenu;
					break;
				}
				case Playing:
				{
					State.Type = Paused;
					break;
				}
				case Paused:
				{
					State.Type = Playing;
					break;
				}
			}
		}

		BeginDrawing();
		switch(State.Type)
		{
			case MainMenu:
			{
				DrawMainMenu();
				break;
			}
			case Controls:
			{
				DrawControlsMenu();
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

					UpdateParticles(&State.Particles, dt);
					UpdateProjectiles(&State.Projectiles, dt);
					UpdateEnemies(&State.Enemies, State.Player.Position, dt);
					UpdatePlayer(&State.Player, dt);
					UpdateGameCamera(&State, dt);

					bool32 Result = ResolveCollisions(&State);
					if(Result)
					{
                        PlaySound(ExplosionSound);
                        ShakeCamera(&State);
					}

					for(int32 i = 0; i < MAX_STARS; i++)
					{
						Stars[i].x -= State.Player.Velocity.x * 0.25f * dt;
						Stars[i].y -= State.Player.Velocity.y * 0.25f * dt;

						Stars[i].x = Wrap(Stars[i].x, 0.0f, (f32)WindowWidth);
						Stars[i].y = Wrap(Stars[i].y, 0.0f, (f32)WindowHeight);
					}

					DrawBackground();
					BeginMode2D(State.Camera);

					DrawPlayer(State.Player, PlayerTexture);
					DrawProjectiles(State.Projectiles);
					DrawEnemies(State.Enemies, EnemyTexture);
					DrawParticles(&State.Particles);

					EndMode2D();
					DrawHUD();
					break;
				}
				case Paused:
				{
					DrawBackground();
					BeginMode2D(State.Camera);

					DrawPlayer(State.Player, PlayerTexture);
					DrawProjectiles(State.Projectiles);
					DrawEnemies(State.Enemies, EnemyTexture);
					DrawParticles(&State.Particles);

					EndMode2D();
					DrawHUD();
					DrawPausedMenu();
					break;
				}
				case GameOver:
		        {
					if(IsSoundPlaying(ThrusterSound))
					{
					    StopSound(ThrusterSound);
					}
					DrawGameOver();
					break;
				}
				case Exit:
				{
					State.IsRunning = false;
					break;
				}
			}
			EndDrawing();
		}

    CloseWindow();
    return(0);
}
