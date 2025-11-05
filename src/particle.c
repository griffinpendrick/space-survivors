#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#define PARTICLE_COUNT 1024

typedef struct
{
    Vector2 Position[PARTICLE_COUNT];
    Vector2 Velocity[PARTICLE_COUNT];
    Color Colors[PARTICLE_COUNT];
    f32 Size[PARTICLE_COUNT];
    f32 Lifetime[PARTICLE_COUNT];
} particle_system;

void UpdateParticles(particle_system* Particles, f32 dt)
{
    for(int32 i = 0; i < PARTICLE_COUNT; i++)
    {
        if(Particles->Lifetime[i] > 0.0f)
        {
            Particles->Position[i] = Vector2Add(Particles->Position[i], Vector2Scale(Particles->Velocity[i], dt));
            Particles->Lifetime[i] -= dt;
        }
    }
}

void EmitParticles(particle_system* Particles, Vector2 Position, Color ParticleColor, int Count)
{
    for(int32 j = 0; j < Count; j++)
    {
        for(int32 i = 0; i < PARTICLE_COUNT; i++)
        {
            if(Particles->Lifetime[i] <= 0.0f)
            {
                f32 Angle = RandomFloat(0.0f, 1.0f) * 2 * PI;
                f32 Speed = RandomFloat(50.0f, 100.0f);
                Particles->Position[i] = (Vector2){Position.x + RandomFloat(-5.0f, 5.0f), Position.y + RandomFloat(-5.0f, 5.0f)};
                Particles->Velocity[i] = (Vector2){cosf(Angle) * Speed, sinf(Angle) * Speed};
                Particles->Colors[i] = ParticleColor;
                Particles->Size[i] = RandomFloat(0.5f, 1.5f);
                Particles->Lifetime[i] = 1.0f;
                break;
            }
        }
    }
}

inline void DrawParticles(particle_system* Particles)
{
    for(int32 i = 0; i < PARTICLE_COUNT; i++)
    {
        if(Particles->Lifetime[i] > 0.0f)
        {
            Particles->Colors[i].a = (uint8)(255.0f * Clamp(Particles->Lifetime[i], 0.0f, 1.0f));
            DrawCircleV(Particles->Position[i], Particles->Size[i], Particles->Colors[i]);
        }
    }
}