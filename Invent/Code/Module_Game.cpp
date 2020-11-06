#include "Module_Game.h"

#include "SDL.h"

constexpr float max_delta_time = 0.05f;

static void poll_input(Game_Control& control)
{
  SDL_GetMouseState(&control.mouse_pos.x, &control.mouse_pos.y);
  SDL_Event sdl_event;
  // move is to was
  SDL_memcpy(&control.was, &control.is, sizeof(control.is));


  while (SDL_PollEvent(&sdl_event))
  {
    switch (sdl_event.type)
    {
    case SDL_QUIT:
    {
      control.is[Key::EXIT] = true;
    } break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
    {
      bool is_down = sdl_event.type == SDL_KEYDOWN ? true : false;
      switch (sdl_event.key.keysym.scancode)
      {
      case SDL_SCANCODE_ESCAPE: { control.is[Key::EXIT] = is_down; } break;
      case SDL_SCANCODE_1: {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      } break;
      case SDL_SCANCODE_2:
      {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }break;
      case SDL_SCANCODE_UP:
      {
        control.is[Key::UP] = is_down;
      } break;
      case SDL_SCANCODE_DOWN:
      {
        control.is[Key::DOWN] = is_down;
      } break;
      case SDL_SCANCODE_LEFT:
      {
        control.is[Key::LEFT] = is_down;
      } break;
      case SDL_SCANCODE_RIGHT:
      {
        control.is[Key::RIGHT] = is_down;
      } break;
      case SDL_SCANCODE_RETURN:
      {
        control.is[Key::SELECT] = is_down;
      }break;
      default:
        break;
      }
    } break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      bool is_down = sdl_event.type == SDL_MOUSEBUTTONDOWN ? true : false;
      if (SDL_BUTTON(sdl_event.button.button) & SDL_BUTTON(SDL_BUTTON_LEFT))
      {
        control.is[Key::LMB] = is_down;
      }
      if (SDL_BUTTON(sdl_event.button.button) & SDL_BUTTON(SDL_BUTTON_RIGHT))
      {
        control.is[Key::RMB] = is_down;
      }
    }break;
    default:
      break;
    }
  }
}

glm::ivec2 Module_Game::get_window_dimension() const
{
  return window_dimension;
}

void Module_Game::start_up()
{
  window_dimension = { 1080, 720 };
  this->is_running = true;
}

void Module_Game::start_frame()
{
  poll_input(game_control);
  while (!SDL_TICKS_PASSED(SDL_GetTicks(), real_tick_elapsed + 16))
  {
    if (!SDL_TICKS_PASSED(SDL_GetTicks(), real_tick_elapsed + 18))
    {
      SDL_Delay(1);
    }
  }
  
  float delta_time = (SDL_GetTicks() - real_tick_elapsed) * 0.001f; // Cast to second
  if (delta_time > max_delta_time) // Prevent to fly away when paused
  {
    delta_time = max_delta_time;
  }
  real_tick_elapsed = SDL_GetTicks();
  if (game_control.is[Key::EXIT] && !game_control.was[Key::EXIT])
  {
    is_running = false;
  }
}

void Module_Game::shut_down()
{
  is_running = false;
}

bool Module_Game::running() const
{
  return is_running;
}
