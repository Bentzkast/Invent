#pragma once
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL.h>

struct Memory_Chunk
{
  size_t capacity;
  size_t used;
  uint8_t* base;

  void init(size_t size);
  void* allocate(size_t size);
  void clear();
};


template <typename T>
struct Array {
  T* data;
  size_t size;
  size_t capacity;

  void Init(Memory_Chunk* memory, size_t count);

  T& operator[](size_t index)
  {
    return data[index];
  }
};


//
//template <typename T,size_t c>
//struct Fixed_Array {
//  T data[c];
//  size_t size;
//  const size_t capacity = c;
//  T& operator[](size_t index)
//  {
//    return data[index];
//  }
//};

struct Game_Input
{
  union
  {
    struct Was
    {
      bool w_down;
      bool s_down;
      bool up_down;
      bool down_down;
      bool exit_down;
      bool enter_down;
    } was;
    bool was_down[6];
  };
  union
  {
    struct Is
    {
      bool w_down;
      bool s_down;
      bool up_down;
      bool down_down;
      bool exit_down;
      bool enter_down;
    } is;
    bool is_down[6];
  };
};

enum class Music {
  EMPTY,
  MAIN_THEME,
  BOSS_THEME
};

struct Audio_Buffer
{
  bool play_tick_sound;
  bool play_bop_sound;
  Music current_music;
};

struct Screen_Text
{
  glm::vec2 pos;
  char* text;
  uint32_t text_len;
  glm::vec4 color_tint;
};

struct Game_Screen
{
  glm::vec2 dimension;
  Array<Screen_Text> screen_texts;

  void push_screen_texts(glm::vec2 pos, char* text);
};

