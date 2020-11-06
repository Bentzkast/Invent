#pragma once

class Module_SDL
{
  using SDL_GLContext = void*;
  
  struct SDL_Window* window = nullptr;
  SDL_GLContext gl_context = nullptr;

public:
  bool start_up(const class Module_Game& game_status);
  void shut_down();
};