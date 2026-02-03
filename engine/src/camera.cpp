#include <engine/camera.hpp>

#include <engine/context.hpp>
#include <engine/sdl.hpp>

namespace gt
{

camera::camera()
    : camera({ 0.0f, 0, 0 }, { 0.0f, 0, -1 }, { 0.0f, 1, 0 })
{
}

camera::camera(vec3 const& pos, vec3 const& dir, vec3 const& up)
    : init_position{ pos }
    , init_direction{ dir }
    , world_up{ up }
    , position{ pos }
    , direction{ dir }
{
    update_vectors();
}


void camera::handle_event(SDL_Event const& ev)
{
    SDL_Window * window = gt::ctx().window;

    switch (ev.type)
    {
        case SDL_EVENT_KEY_DOWN:
        {
            switch (ev.key.key)
            {
            case SDLK_W:
                is_w = true;
                break;
            case SDLK_S:
                is_s = true;
                break;
            case SDLK_A:
                is_a = true;
                break;
            case SDLK_D:
                is_d = true;
                break;
            case SDLK_EQUALS:
                position = init_position;
                yaw = init_yaw;
                pitch = init_pitch;
                update_vectors();
                break;
            }

            is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;

            bool const new_is_lctrl = ev.key.mod & SDL_KMOD_LCTRL;
            if (is_lctrl != new_is_lctrl)
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);
                is_lctrl = new_is_lctrl;
            }
        }
        break;

        case SDL_EVENT_KEY_UP:
        {
            switch (ev.key.key)
            {
            case SDLK_W:
                is_w = false;
                break;
            case SDLK_S:
                is_s = false;
                break;
            case SDLK_A:
                is_a = false;
                break;
            case SDLK_D:
                is_d = false;
                break;
            }

            is_lshift = ev.key.mod & SDL_KMOD_LSHIFT;

            bool const new_is_lctrl = ev.key.mod & SDL_KMOD_LCTRL;
            if (is_lctrl != new_is_lctrl)
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                SDL_WarpMouseInWindow(window, f32(w) / 2, f32(h) / 2);
                is_lctrl = new_is_lctrl;
            }
        }
        break;

        case SDL_EVENT_MOUSE_MOTION:
        {
            if (is_lctrl)
            {
                if (!SDL_SetWindowRelativeMouseMode(window, false))
                    sdl::log_error();
                SDL_ShowCursor();
            }
            else
            {
                if (!SDL_SetWindowRelativeMouseMode(window, true))
                    sdl::log_error();
                SDL_HideCursor();

                pitch -= ev.motion.yrel * 0.05f;
                yaw += ev.motion.xrel * 0.05f;

                pitch = std::clamp(pitch, -89.0f, 89.0f);

                update_vectors();
            }
        }
        break;
    }
}

void camera::simulate(f32 d)
{

    f32 const real_speed = speed * (is_lshift ? 4.0f : 1.0f);
    if (is_w)
        position += d * real_speed * direction;
    if (is_s)
        position -= d * real_speed * direction;
    if (is_a)
        position -= right * d * real_speed;
    if (is_d)
        position += right * d * real_speed;
}

mat4 camera::view() const
{
    return lookAt(position, position + direction, up);
}

void camera::update_vectors()
{
    direction[0] = std::cos(radians(pitch)) * std::cos(radians(yaw));
    direction[1] = std::sin(radians(pitch));
    direction[2] = std::cos(radians(pitch)) * std::sin(radians(yaw));

    direction = normalize(direction);
    right = normalize(cross(direction, world_up));
    up = normalize(cross(right, direction));
}

}
