#define ROTATION_SPEED 300.0f
#define FRICTION_COEFFICIENT 0.98f

typedef struct player player;
struct player
{
    Vector2 Position;
    Vector2 Velocity;
    Vector2 Acceleration;
    f32 Angle;
    f32 Health;
    f32 MaxHealth;
    f32 DamageCooldown;
    f32 Exp;
    int32 ProjectileCount;
    int32 Level;
    bool32 HasLeveledUp;
};

INLINE f32 ExpRequiredForLevelUp(int32 CurrentLevel)
{
    return(floorf(150.0f * powf(1.18f, CurrentLevel - 1.0f)));
}

void AcceleratePlayer(player* Player, f32 dt)
{
    f32 Radians = Player->Angle * DEG2RAD;
    Vector2 Direction = (Vector2){sinf(Radians), -cosf(Radians)};
    Player->Velocity.x += Direction.x * Player->Acceleration.x * dt;
    Player->Velocity.y += Direction.y * Player->Acceleration.y * dt;
}

void UpdatePlayer(player* Player, f32 dt)
{
    if(Player->Exp >= ExpRequiredForLevelUp(Player->Level))
    {
        Player->Level++;
        Player->HasLeveledUp = true;
        Player->Exp = 0.0f;
    }

    Player->Velocity = Vector2Lerp(Player->Velocity, (Vector2){0.0f, 0.0f}, FRICTION_COEFFICIENT * dt);
    Player->Position = Vector2Add(Player->Position, Vector2Scale(Player->Velocity, dt));

    Player->Position.x = Wrap(Player->Position.x, -15.0f, WINDOW_WIDTH + 15.0f);
    Player->Position.y = Wrap(Player->Position.y, -15.0f, WINDOW_HEIGHT + 15.0f);

    Player->DamageCooldown -= dt;
}

void DrawPlayer(player Player)
{
    DrawTexturePro(PlayerTexture,
        (Rectangle){0.0f, 0.0f, (f32)PlayerTexture.width, (f32)PlayerTexture.height},
        (Rectangle){Player.Position.x, Player.Position.y, (f32)PlayerTexture.width, (f32)PlayerTexture.height},
        (Vector2){PlayerTexture.width / 2.0f, PlayerTexture.height / 2.0f},
        Player.Angle,
        WHITE);
}
