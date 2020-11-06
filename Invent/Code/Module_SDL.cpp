#include "Module_SDL.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <glad/glad.h>

#include "Module_Game.h"

bool Module_SDL::start_up(const Module_Game& game)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 < 0))
  {
    SDL_Log("Unable to initialize SDL mixer: %s", Mix_GetError());
    return false;
  }
  SDL_ShowCursor(0);
  auto window_dim = game.get_window_dimension();
  window = SDL_CreateWindow(
    "Micro Realm",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    window_dim.x,
    window_dim.y,
    SDL_WINDOW_OPENGL
  );
  if (window == NULL)
  {
    SDL_Log("Unable to create window: %s", SDL_GetError());
    return false;
  }

  // Context Profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // Versions
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  // Color Buffer
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  // Enable double buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Force OpenGL to use hardware acceleration
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  gl_context = SDL_GL_CreateContext(window);
  if (gl_context == NULL)
  {
    SDL_Log("Unable to create gl context: %s", SDL_GetError());
    return false;
  }

  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    SDL_Log("Unable to create gl context: %s", SDL_GetError());
    return false;
  }

  SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
  SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
  SDL_Log("Version:  %s", glGetString(GL_VERSION));

  SDL_GL_SetSwapInterval(1); // V SYNC
  return true;
}

void Module_SDL::shut_down()
{
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  Mix_Quit();
  SDL_Quit();
}
