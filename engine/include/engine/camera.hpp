#pragma once

#include <engine/math.hpp>
#include <engine/fundamental.hpp>

#include <SDL3/SDL_events.h>

namespace gt
{

struct camera
{
    camera();
    camera(vec3 const& pos, vec3 const& dir, vec3 const& up);

    void handle_event(SDL_Event const& ev);
    void simulate(f32 d);
    mat4 view() const;

private:
    void update_vectors();

private:
    vec3 init_position;
    vec3 init_direction;
    f32 speed = 0.3f;
    vec3 world_up{};

    vec3 position{};
    vec3 direction{};

    vec3 right{};
    vec3 up{};

    bool is_w{ false };
    bool is_s{ false };
    bool is_a{ false };
    bool is_d{ false };
    bool is_lshift{ false };
    bool is_lctrl{ false };

    f32 init_pitch{ 0 };
    f32 init_yaw{ -90 };

    f32 pitch{ init_pitch };
    f32 yaw{ init_yaw };
};

}
