#pragma once

#include "app.h"

const char *TITLE = "imgui_template_app";
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const SDL_WindowFlags WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

void Build();