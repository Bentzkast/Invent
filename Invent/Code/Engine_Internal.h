#pragma once
#include "Engine.h"
#include <SDL_mixer.h>
#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// @TODO
// Switch to gl4?

struct SDL_Context
{
  SDL_Window* window = nullptr;
  int window_width = 0;
  int window_height = 0;
  SDL_GLContext gl_context = nullptr;

  bool create();
  int destroy();
};

struct Audio_Library {
  Mix_Chunk* impact_sound;
  Mix_Chunk* bop_sound;
  Mix_Music* main_theme;
  Mix_Music* boss_theme;
  Music current_music;

  bool load_all();
};

struct Character {
  glm::vec2   size;      // Size of glyph
  glm::vec2   bearing;   // Offset from baseline to left/top of glyph
  unsigned int advance;   // Horizontal offset to advance to next glyph
  glm::vec2	normalized_uv;  // UV pos  in the texture
};

struct Font_Character
{
  Character characters_table[128]; // @Important 128 since it mapped to ascii not wort the complexities off densely packing these
  uint32_t gl_texture;
  int font_texture_dimension_px;
};

struct Sprite_Sheet
{
  uint32_t gl_texture;
  int width;
  int height;
  float resolution;

  bool load_sprite_sheet(const char* filepath);
};

struct Game_Memory
{
  size_t capacity;
  size_t used;
  uint8_t* base;

  void init(size_t size);
  void* allocate(size_t size);
  void clear();
  void free();
};