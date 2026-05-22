#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WAVES_MAX_SIZE 2048

// Assuming: RRGGBBAA
#define HEX_TO_COLOR(x) (Color) { .r = ((x) >> 8 * 3) & 0xff, .g = ((x) >> 8 * 2) & 0xff, .b = ((x) >> 8 * 1) & 0xff, .a = ((x) >> 8 * 0) & 0xff }

#define BACKGROUND_COLOR HEX_TO_COLOR(0x0e1219aa)

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float radius;
    Color color;
} Particle;

typedef struct {
    Vector2 center;
    float velocity;
    float radius;
    Color color;
    float phase;
} WaveForm;

size_t wave_count = 0;
static WaveForm waves[WAVES_MAX_SIZE] = { 0 };

void update_particle(Particle* particle, float dt) {
    Vector2 ut = Vector2Scale(particle->velocity, dt);
    particle->position = Vector2Add(
        particle->position,
        Vector2Add(ut, Vector2Scale(particle->acceleration, 0.5 * dt * dt))
    );
    particle->velocity = Vector2Add(particle->velocity, Vector2Scale(particle->acceleration, dt));
}

void emit_waveform(Particle* particle) {
    if (wave_count >= WAVES_MAX_SIZE) return;
    WaveForm wave = {
        .center = particle->position,
        .velocity = 100.0f,
        .radius = particle->radius,
        .color = HEX_TO_COLOR(0x7f8ca1aa),
        .phase = 0.0f
    };
    waves[wave_count++] = wave;
}

void draw_waveform(WaveForm* wave) {
    DrawCircleLinesV(wave->center, wave->radius + wave->phase, wave->color);
}

int32_t main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Doppler");
    Particle particle = {
        .position = {
            .x = 0,
            .y = WINDOW_HEIGHT / 2
        },
        .velocity = {
            .x = 10.0f,
            .y = 0.0f
        },
        .acceleration = {
            .x = 10.0f,
            .y = 0.0f
        },
        .radius = 5.0f,
        .color = HEX_TO_COLOR(0xfe166daa)
    };

    SetTargetFPS(60);
    float time_elapsed = 0.0f;
    const float delay = 0.1f;
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        time_elapsed += dt;
        update_particle(&particle, dt);
        if (time_elapsed >= delay) {
            emit_waveform(&particle);
            time_elapsed = 0;
        }
        for (size_t i = 0; i < wave_count; ++i) {
            waves[i].radius += waves[i].velocity * dt;
        }
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            DrawCircleV(particle.position, particle.radius, particle.color);
            for (size_t i = 0; i < wave_count; ++i) {
                draw_waveform(&waves[i]);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
