#include "Engine_Internal.h"

bool SDL_Context::create()
{
  window_width = 1080;
  window_height = 720;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 < 0))
  {
    SDL_Log("Unable to initialize SDL mixer: %s", Mix_GetError());
    return false;
  }

  window = SDL_CreateWindow(
    "PONG",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    window_width,
    window_height,
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

int SDL_Context::destroy()
{
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  Mix_Quit();
  SDL_Quit();
  return 0;
}

static void poll_input(Game_Input* game_input)
{
  SDL_Event sdl_event;
  // move was to is
  memcpy_s(&game_input->was, sizeof(game_input->was), &game_input->is, sizeof(game_input->is));

  while (SDL_PollEvent(&sdl_event))
  {
    switch (sdl_event.type)
    {
    case SDL_QUIT:
    {
      game_input->is.exit_down = true;
    } break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
    {
      bool is_down = sdl_event.type == SDL_KEYDOWN ? true : false;
      switch (sdl_event.key.keysym.scancode)
      {
      case SDL_SCANCODE_ESCAPE:
      {
        game_input->is.exit_down = true;
      }break;
      case SDL_SCANCODE_1:
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      } break;
      case SDL_SCANCODE_2:
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }break;
      case SDL_SCANCODE_3:
      {

      }break;
      case SDL_SCANCODE_W:
      {
        game_input->is.w_down = is_down;
        //SDL_Log("SDL_SCANCODE_W %s", is_s_down ? "true" : "false");
      } break;
      case SDL_SCANCODE_S:
      {
        game_input->is.s_down = is_down;
        //SDL_Log("SDL_SCANCODE_S %s", is_s_down ? "true" : "false");
      } break;
      case SDL_SCANCODE_UP:
      {
        game_input->is.up_down = is_down;
      } break;
      case SDL_SCANCODE_DOWN:
      {
        game_input->is.down_down = is_down;
      } break;
      case SDL_SCANCODE_LEFT:
      {
        game_input->is.left_down = is_down;
      } break;
      case SDL_SCANCODE_RIGHT:
      {
        game_input->is.right_down = is_down;
      } break;
      case SDL_SCANCODE_RETURN:
      {
        game_input->is.enter_down = is_down;
      }break;
      default:
        break;
      }
    } break;
    default:
      break;
    }
  }
}

static Uint32 load_shader_2d(const char* vertex_shader_source, const char* fragment_shader_source)
{
  Uint32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  int success;
  char temp_info_log_buffer[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertex_shader, 512, NULL, temp_info_log_buffer);
    SDL_Log("%s", temp_info_log_buffer);
  }

  Uint32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragment_shader, 512, NULL, temp_info_log_buffer);
    SDL_Log("%s", temp_info_log_buffer);
  }

  // link shaders
  Uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);

  glAttachShader(shader_program, fragment_shader);

  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader_program, 512, NULL, temp_info_log_buffer);
    SDL_Log("%s", temp_info_log_buffer);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

bool Audio_Library::load_all()
{
  bool result = true;
  impact_sound = Mix_LoadWAV("Resources/Sound/impact.ogg");
  if (!impact_sound)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
    result = false;
  }
  bop_sound = Mix_LoadWAV("Resources/Sound/bong.ogg");
  if (!bop_sound)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
    result = false;
  }

  main_theme = Mix_LoadMUS("Resources/Music/Resilience.ogg");
  if (!main_theme)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
    result = false;
  }

  boss_theme = Mix_LoadMUS("Resources/Music/Battleship.ogg");
  if (!boss_theme)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
    result = false;
  }

  return result;
}

bool Sprite_Sheet::load_sprite_sheet(const char* filepath)
{
  // @NOTE since the matrix projection is flip along y to top left coordinate, dont need to flip it here.
  //stbi_set_flip_vertically_on_load(true);

  int color_channel = 0;;
  Uint8* image_data = stbi_load(filepath, &width, &height, &color_channel, 0);
  if (!image_data)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find image file : %s", filepath);
    return false;
  }

  // Create GL texture
  glGenTextures(1, &gl_texture);
  glBindTexture(GL_TEXTURE_2D, gl_texture);
  // texture wrapping
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  int format = GL_RGB;
  if (color_channel == 4)
  {
    format = GL_RGBA;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(image_data);

  // @TEMP hmmm
  resolution = 16.0f;

  return true;
}
