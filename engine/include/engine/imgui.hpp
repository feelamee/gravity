#pragma once

union SDL_Event;

namespace gt::im
{

void handle_event(SDL_Event const&);
void frame();
void render();

}

