
#include <SDL_mixer.h>
#include <glad/glad.h>

#include "Platform.h"

#include "Game.cpp"
// Target "PONG" with opengl sound menu particles etc ... (complete game thats it)
// ONLY documentation!!

// TODO
// Switch to gl4?

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"


#include <ft2build.h>
#include FT_FREETYPE_H

template <typename T>
void Array<T>::Init(Memory_Chunk* memory, size_t count)
{
 capacity = count;
 size = 0;
 data = (T*)(memory->allocate(sizeof(T) * 50));
}

void Game_Screen::push_screen_texts(glm::vec2 pos, char* text)
{
  SDL_assert((screen_texts.size + 1) <= screen_texts.capacity);
  screen_texts[screen_texts.size].pos = pos;
  screen_texts[screen_texts.size].text = text;
  screen_texts[screen_texts.size].text_len = SDL_strlen(text);
  screen_texts.size++;
}

struct SDL_Context
{
  SDL_Window* window = nullptr;
  int window_width = 0;
  int window_height = 0;
  SDL_GLContext gl_context = nullptr;

  bool create()
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

  int destroy()
  {
      SDL_GL_DeleteContext(gl_context);
      SDL_DestroyWindow(window);
      Mix_CloseAudio();
      SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
      Mix_Quit();
      SDL_Quit();
      return 0;
  }
};

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

static Uint32 load_shader_2d(const char * vertex_shader_source, const char* fragment_shader_source)
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


void Memory_Chunk::init(size_t size)
{
  base = (Uint8*)SDL_calloc(1, size); // 1 mega bytes
  capacity = size;
}

void* Memory_Chunk::allocate(size_t size)
{
  SDL_assert((used + size) <= capacity);
  void* result = base + used;
  used += size;
  return result;
}

void Memory_Chunk::clear()
{
  SDL_memset(base, 0, used);
  used = 0;
}

struct Vertex_2D
{
  glm::vec2 pos;
  glm::vec2 uv;
};

struct Quad_Verts
{
  Vertex_2D top_left;
  Vertex_2D bot_left;
  Vertex_2D top_right;
  Vertex_2D bot_right;
};
static void entity_to_quad_verts(const Entity_2D* entity, Quad_Verts* quadVert)
{
  quadVert->top_left = { { entity->pos.x,  entity->pos.y }, {0,0} };
  quadVert->bot_left = { { entity->pos.x,  entity->pos.y + entity->size.y  }, {0,0} };
  quadVert->top_right = { { entity->pos.x + entity->size.x,  entity->pos.y  } , {0,0} };
  quadVert->bot_right = { { entity->pos.x + entity->size.x,  entity->pos.y + entity->size.y  }, {0,0} };
}

struct Mesh {
  Uint32 vertex_array;
  Uint32 vertex_buffer;
  Uint32 element_buffer;
};

struct Quad_Triangle
{
  Uint32 one;
  Uint32 two;
  Uint32 three;
  Uint32 four;
  Uint32 five;
  Uint32 six;
  static const int element_count = 6;
};

static void create_2d_mesh(Memory_Chunk* temp_memory, int entity_count, Mesh* mesh)
{
  glGenVertexArrays(1, &mesh->vertex_array);
  glGenBuffers(1, &mesh->vertex_buffer);
  glGenBuffers(1, &mesh->element_buffer);

  glBindVertexArray(mesh->vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

  // @TODO proper array data struct
  Quad_Triangle* quad_triangles_array = (Quad_Triangle*)temp_memory->allocate(entity_count * sizeof(Quad_Triangle));
  for (int i = 0; i < entity_count; i++)
  {
    quad_triangles_array[i].one = 0 + i * 4;
    quad_triangles_array[i].two = 1 + i * 4;
    quad_triangles_array[i].three = 2 + i * 4;

    quad_triangles_array[i].four = 1 + i * 4;
    quad_triangles_array[i].five = 3 + i * 4;
    quad_triangles_array[i].six = 2 + i * 4;
  }

  // set input data
  glBufferData(GL_ARRAY_BUFFER, sizeof(Quad_Verts) * entity_count, 0, GL_DYNAMIC_DRAW); // entities size here
  // @Important this use size , draw use count
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(Quad_Triangle) * entity_count, quad_triangles_array, GL_STATIC_DRAW);

  // Push first layout
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2D), (void*)(offsetof(Vertex_2D, pos)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2D), (void*)(offsetof(Vertex_2D, uv)));
  glEnableVertexAttribArray(1);
}


static void destroy_2d_mesh(Mesh* mesh)
{
  SDL_Log("Destroying Mesh");
  glDeleteBuffers(1, &mesh->vertex_array);
  glDeleteBuffers(1, &mesh->vertex_buffer);
  glDeleteBuffers(1, &mesh->element_buffer);
  mesh->vertex_array = 0;
  mesh->vertex_buffer = 0;
  mesh->element_buffer = 0;
}

struct Character {
  glm::vec2   size;      // Size of glyph
  glm::vec2   bearing;   // Offset from baseline to left/top of glyph
  unsigned int advance;   // Horizontal offset to advance to next glyph
  glm::vec2	normalized_uv;  // UV pos  in the texture
};

// @TODO move to utils;
static inline int max_value(int a, int b)
{
  return a > b ? a : b;
}

struct Font_Character
{
  Character characters_table[128]; // @Important 128 since it mapped to ascii not wort the complexities off densely packing these
  uint32_t gl_texture;
  int font_texture_dimension_px;
};

static bool font_character_create(const char* filepath, const int font_size, Font_Character* font_character)
{
  FT_Library ft;
  if (FT_Init_FreeType(&ft))
  {
    SDL_Log("Failed to ini free type");
    return false;
  }

  FT_Face face;
  if (FT_New_Face(ft, filepath, 0, &face))
  {
    SDL_Log("failed to load font");

    return false;
  }
  FT_Set_Pixel_Sizes(face, 0, font_size);
  font_character->font_texture_dimension_px = 1024;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &font_character->gl_texture);
  glBindTexture(GL_TEXTURE_2D, font_character->gl_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font_character->font_texture_dimension_px, font_character->font_texture_dimension_px, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int current_row_max_height = 0;
  int current_total_height = 0;
  int current_x = 0;
  int current_y = 0;
  // Create Font information
  for (unsigned char c = 32; c < 128; c++)
  {
    // Load character glyph 
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
      SDL_Log("Error loading font for character: %c", c);
      continue;
    }

    current_row_max_height = max_value(current_row_max_height, (int)face->glyph->bitmap.rows);
    if ((current_x + face->glyph->bitmap.width) > font_character->font_texture_dimension_px)
    {
      // Move to next row
      current_y += current_row_max_height;
      SDL_assert(current_y < font_character->font_texture_dimension_px);

      current_row_max_height = 0;
      current_x = 0;
    }

    glTexSubImage2D(
      GL_TEXTURE_2D, 0,
      (GLint)current_x, (GLint)current_y,
      (GLint)face->glyph->bitmap.width, (GLint)face->glyph->bitmap.rows,
      GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    font_character->characters_table[c].size = glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    font_character->characters_table[c].bearing = glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    font_character->characters_table[c].advance = face->glyph->advance.x;
    font_character->characters_table[c].normalized_uv = glm::vec2(current_x / (float)font_character->font_texture_dimension_px, current_y / (float)font_character->font_texture_dimension_px);

    current_x += face->glyph->bitmap.width;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore default 

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}


static void screen_text_to_quad_verts(const Font_Character* font_characters, const Screen_Text* screen_text, Quad_Verts* quad_vertices)
{
  int text_pos_x = screen_text->pos.x;
  for (int i = 0; i < screen_text->text_len; i++)
  {
    const Character* ch = &font_characters->characters_table[screen_text->text[i]];

    float xpos = text_pos_x + ch->bearing.x;
    float ypos = screen_text->pos.y + font_characters->characters_table['H'].bearing.y - ch->bearing.y;

    float w = ch->size.x;
    float h = ch->size.y;

    float uv_w = ch->size.x / font_characters->font_texture_dimension_px;
    float uv_h = ch->size.y / font_characters->font_texture_dimension_px;
    quad_vertices[i].top_left.pos = glm::vec2{ xpos,     ypos };
    quad_vertices[i].top_left.uv = glm::vec2{ ch->normalized_uv.x, ch->normalized_uv.y };

    quad_vertices[i].bot_left.pos = glm::vec2{ xpos,     ypos + h };
    quad_vertices[i].bot_left.uv = glm::vec2{ ch->normalized_uv.x, ch->normalized_uv.y + uv_h };

    quad_vertices[i].top_right.pos = glm::vec2{ xpos + w, ypos };
    quad_vertices[i].top_right.uv = glm::vec2{ ch->normalized_uv.x + uv_w, ch->normalized_uv.y };

    quad_vertices[i].bot_right.pos = glm::vec2{ xpos + w, ypos + h };
    quad_vertices[i].bot_right.uv = glm::vec2{ ch->normalized_uv.x + uv_w, ch->normalized_uv.y + uv_h };

    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    text_pos_x += (ch->advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
  }
}



struct Audio_Library {
  Mix_Chunk* impact_sound;
  Mix_Chunk* bop_sound;
  Mix_Music* main_theme;
  Mix_Music* boss_theme;
  Music current_music;
};

int main(int argc,char* argv[])
{
  SDL_Context sdl_context{};
  if (!sdl_context.create())
  {
    sdl_context.destroy();
    return 0;
  }

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_CULL_FACE); // Face culling
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Memory_Chunk main_memory{};
  main_memory.init(1 * 1024 * 1024); // 1 mb
  Memory_Chunk frame_memory{};
  frame_memory.init(1 * 1024 * 1024); // 1 mb

  glm::mat4 projection_matrix = glm::ortho(0.0f, (float)sdl_context.window_width, (float)sdl_context.window_height, 0.0f);

  const char* vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "layout (location = 1) in vec2 a_uv;"
    "uniform mat4 u_projection_mat;"
    "void main()"
    "{"
    "   gl_Position = u_projection_mat * vec4(a_pos.x, a_pos.y, 0.0, 1.0);"
    "}";
  const char* fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;"
    "void main()"
    "{"
    "   FragColor = vec4(0.9f, 0.9f, 0.9f, 1.0f);" // WHITE
    "}";

  Uint32 shader_program = load_shader_2d(vertex_shader_source, fragment_shader_source);
  SDL_Log("%d", glGetError());

  const char* font_vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "layout (location = 1) in vec2 a_uv;"
    "out vec2 v_tex_coord;"
    "uniform mat4 u_projection_mat;"
    "void main()"
    "{"
    "   gl_Position = u_projection_mat * vec4(a_pos.x, a_pos.y, 0.0, 1.0);"
    "   v_tex_coord = a_uv;"
    "}";
  const char* font_fragment_shader_source = "#version 330 core\n"
    "in vec2 v_tex_coord;"
    "out vec4 FragColor;"
    "uniform vec4 u_tint_color;"
    "uniform sampler2D u_texture;"
    "void main()"
    "{"
    "   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_texture, v_tex_coord).r);" // only sample alpha
    "   FragColor = u_tint_color * sampled;" // WHITE
    "}";

  Uint32 font_shader = load_shader_2d(font_vertex_shader_source, font_fragment_shader_source);

  constexpr int max_entity_count = 100;
  Mesh quad_mesh;
  create_2d_mesh(&frame_memory, max_entity_count, &quad_mesh);
  frame_memory.clear();



  // @TODO use game memory, use stack for now
  Game_Input game_input = {};
  Font_Character liberation_mono_font = {};
  font_character_create("Resources/Font/liberation_mono.ttf", 40, &liberation_mono_font);


  // AUDIO
  // @TODO music library module
  Audio_Library* audio_library = (Audio_Library *) main_memory.allocate(sizeof(Audio_Library));
  audio_library->impact_sound = Mix_LoadWAV("Resources/Sound/impact.ogg");
  if (!audio_library->impact_sound)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
  }
  audio_library->bop_sound = Mix_LoadWAV("Resources/Sound/bong.ogg");
  if (!audio_library->bop_sound)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
  }

  audio_library->main_theme = Mix_LoadMUS("Resources/Music/Resilience.ogg");
  if (!audio_library->main_theme)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
  }

  audio_library->boss_theme = Mix_LoadMUS("Resources/Music/Battleship.ogg");
  if (!audio_library->boss_theme)
  {
    SDL_Log("Unable to load music file %s", Mix_GetError());
  }


  // @TODO use game memory, use stack for now
  Game_State* game_state = (Game_State*)main_memory.allocate(sizeof(Game_State));
  Audio_Buffer audio_buffer{ 0 };
  game_state->setup(&main_memory, &audio_buffer);
  int volume = MIX_MAX_VOLUME * 0.05f;
  // @TODO load from setting
  Mix_VolumeMusic(volume);
  //// @TODO collapse this to handle music
  if (audio_buffer.current_music != audio_library->current_music)
  {
    constexpr int play_forever = -1;
    switch (audio_buffer.current_music)
    {
    case Music::EMPTY: {
      Mix_FadeOutMusic(1000);
    }break;
    case Music::MAIN_THEME: {
      if (Mix_PlayingMusic()) { Mix_FadeOutMusic(500); }

      if (Mix_FadeInMusic(audio_library->main_theme, play_forever, 1000) == -1)
      {
        // @TODO better logging
        SDL_Log("ERROR:Mix_FadeInMusic: %s\n", Mix_GetError());
      }
    }break;
    case Music::BOSS_THEME: {
      if (Mix_PlayingMusic()) { Mix_FadeOutMusic(500); }
      if (Mix_FadeInMusic(audio_library->boss_theme, play_forever, 1000) == -1)
      {
        SDL_Log("ERROR::Mix_FadeInMusic: %s\n", Mix_GetError());
      }
    }break;
    default:
      break;
    }
  }

  bool game_is_running = true;
  Uint32 realTickElapsed = SDL_GetTicks();
  constexpr float deltaTimeLimit = 0.05f;

  while (game_state->is_running)
  {
    // For graphics
    // @TODO use game memory, use stack for now
    Quad_Verts quad_vertices[max_entity_count] = {};

    frame_memory.clear();
    poll_input(&game_input);
   
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), realTickElapsed + 16))
    {
      if (!SDL_TICKS_PASSED(SDL_GetTicks(), realTickElapsed + 18))
      {
        SDL_Delay(1);
      }
    }
    // Frame Upper Cap 60
    float delta_time = (SDL_GetTicks() - realTickElapsed) * 0.001f; // Cast to second
    if (delta_time > deltaTimeLimit) // Prevent to fly away when paused
    {
      delta_time = deltaTimeLimit;
    }
    realTickElapsed = SDL_GetTicks();
    delta_time = .0166f;
    
    Audio_Buffer* audio_buffer = (Audio_Buffer*)frame_memory.allocate(sizeof(Audio_Buffer));
    Game_Screen* game_screen = (Game_Screen*)frame_memory.allocate(sizeof(Game_Screen));

    game_screen->dimension = { sdl_context.window_width, sdl_context.window_height };
    game_screen->screen_texts.Init(&frame_memory, 50);
    game_state->update(&main_memory, &frame_memory, delta_time, &game_input,
      audio_buffer, game_screen);

    // handle SFX
    if (audio_buffer->play_tick_sound)
    {
      Mix_PlayChannel(-1, audio_library->impact_sound, 0);
    }   
    if (audio_buffer->play_bop_sound)
    {
      Mix_PlayChannel(-1, audio_library->bop_sound, 0);
    }

    // Render Stuff
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    // Entity Batch
    glBindVertexArray(quad_mesh.vertex_array);
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection_mat"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    SDL_assert(game_state->entities->size <= max_entity_count);

    for (int i = 0; i < game_state->entities->size; i++)
    {
      entity_to_quad_verts(&game_state->entities->data[i], &quad_vertices[i]);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Quad_Verts) * game_state->entities->size, quad_vertices);
    glDrawElements(GL_TRIANGLES, Quad_Triangle::element_count * game_state->entities->size, GL_UNSIGNED_INT, 0);
    
    // Font batch
    glUseProgram(font_shader);
    glUniformMatrix4fv(glGetUniformLocation(font_shader, "u_projection_mat"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniform4f(glGetUniformLocation(font_shader, "u_tint_color"), 0.9f, 0.9f, 0.9f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, liberation_mono_font.gl_texture);
    
    // @READ instancing for approach to handle text with different property like color ??
    // @TODO for now we can pass it in as an vertex attribute ?? 
    int text_width_acc = 0;
    for (int i = 0; i < game_screen->screen_texts.size; i++)
    {
      screen_text_to_quad_verts(&liberation_mono_font, &game_screen->screen_texts[i], quad_vertices + text_width_acc);
      text_width_acc += game_screen->screen_texts[i].text_len;
    }
    SDL_assert(text_width_acc <= max_entity_count);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Quad_Verts)* (text_width_acc), quad_vertices);
    glDrawElements(GL_TRIANGLES, Quad_Triangle::element_count * (text_width_acc), GL_UNSIGNED_INT, 0);

   SDL_GL_SwapWindow(sdl_context.window);
  }


  Mix_FreeChunk(audio_library->impact_sound);
  Mix_FreeMusic(audio_library->main_theme);
  Mix_FreeMusic(audio_library->boss_theme);

   
  destroy_2d_mesh(&quad_mesh);
  glDeleteProgram(shader_program);
  glDeleteProgram(font_shader);
  glDeleteTextures(1, &liberation_mono_font.gl_texture);
  SDL_free(main_memory.base);
  SDL_free(frame_memory.base);
  return sdl_context.destroy();
}