#include <stdio.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WAVES_MAX_SIZE 2048
#define TRAIL_MAX_SIZE 60

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

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

typedef struct {
    Particle particle;
    float time_elapsed;
    float trail_time_elapsed;
    float delay;
    float trail_delay;
} AppState;

size_t trail_count = 0;
static Particle trail[TRAIL_MAX_SIZE] = { 0 };


void add_particle_to_trail(Particle particle) {
    // Acts like a ring buffer
    trail[trail_count++ % TRAIL_MAX_SIZE] = particle;
}

void render_trail(float fade_factor) {
    size_t trail_len = (trail_count < TRAIL_MAX_SIZE) ? trail_count : TRAIL_MAX_SIZE;
    if (trail_len <= 0) return;

    size_t start = (trail_count >= TRAIL_MAX_SIZE) ? trail_count % TRAIL_MAX_SIZE : 0;

    for (size_t age = 0; age < trail_len; ++age) {
        size_t index = (start + age) % TRAIL_MAX_SIZE;
        Particle particle = trail[index];
        float fade = (age + 1) / (float) trail_len;
        DrawCircleV(
            particle.position,
            particle.radius,
            (Color) {
                .r = particle.color.r,
                .g = particle.color.g,
                .b = particle.color.b,
                .a = (uint8_t) ((float) particle.color.a * fade * fade_factor)
            }
        );
    }
}

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

void update_waves(float dt) {
    for (size_t i = 0; i < wave_count; ++i) {
        waves[i].radius += waves[i].velocity * dt;
    }
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

void reset_app_state(AppState* state) {
    state->particle.position = (Vector2){ 0, WINDOW_HEIGHT / 2 };
    state->particle.velocity = (Vector2){ 10.0f, 0.0f };
    state->particle.acceleration = (Vector2){ 10.0f, 0.0f };

    state->time_elapsed = 0.0f;
    state->trail_time_elapsed = 0.0f;

    wave_count = 0;
    trail_count = 0;
}

void update_and_draw(void* arg) {
    AppState* state = arg;
    const float dt = GetFrameTime();
    state->time_elapsed += dt;
    state->trail_time_elapsed += dt;

    update_particle(&state->particle, dt);
    if (state->particle.position.x >= WINDOW_WIDTH + 50) reset_app_state(state);
    if (state->time_elapsed >= state->delay) {
        emit_waveform(&state->particle);
        add_particle_to_trail(state->particle);
        state->time_elapsed = 0;
    }

    if (state->trail_time_elapsed >= state->trail_delay) {
        add_particle_to_trail(state->particle);
        state->trail_time_elapsed = 0;
    }
    update_waves(dt);
    BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        render_trail(0.2f);
        DrawCircleV(state->particle.position, state->particle.radius, state->particle.color);
        for (size_t i = 0; i < wave_count; ++i) {
            draw_waveform(&waves[i]);
        }
    EndDrawing();
}

int32_t main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
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

    static AppState state = { 0 };
    state.particle = particle;
    state.time_elapsed = 0.0f;
    state.trail_time_elapsed = 0.0f;
    state.delay = 0.1f;
    state.trail_delay = 0.03f;

    SetTargetFPS(120);
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(update_and_draw, &state, 0, 1);
#else
    while (!WindowShouldClose()) {
        update_and_draw(&state);
    }
#endif
    CloseWindow();
    return 0;
}
