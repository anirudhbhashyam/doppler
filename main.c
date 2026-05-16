#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WAVES_MAX_SIZE 1024

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
size_t emitted_count = 0;
static WaveForm waves[WAVES_MAX_SIZE];

void update_particle(Particle* particle, float dt) {
    Vector2 ut = Vector2Scale(particle->velocity, dt);
    particle->position = Vector2Add(
        particle->position,
        Vector2Add(ut, Vector2Scale(particle->acceleration, 0.5 * dt * dt))
    );
    particle->velocity = Vector2Add(particle->velocity, ut);
}

void emit_waveform(Particle* particle) {
    WaveForm wave = {
        .center = particle->position,
        .velocity = 200.0f,
        .radius = particle->radius,
        .color = HEX_TO_COLOR(0x7f8ca1aa),
        .phase = wave_count + 2.0f,
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
            .x = 100.0f,
            .y = 0.0f
        },
        .acceleration = {
            .x = 0.01f,
            .y = 0.0f
        },
        .radius = 10.0f,
        .color = HEX_TO_COLOR(0xfe166daa)
    };

    SetTargetFPS(60);
    float time_elapsed = 0.0f;
    const float delay = 0.2f;
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        time_elapsed += dt;
        update_particle(&particle, dt);
        printf("particle position: %f, %f\n", particle.position.x, particle.position.y);
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
    return 0;
}
