#pragma once
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL.h>

// Function Type...
struct Game_Memory
{
  struct Memory_Chunk* memory_chunk;
  //void* push(size_t size);
  template <typename T>
  T* push();
  template <typename T>
  T* push_array(size_t count);
};

template <typename T>
struct Array {
  T* data;
  size_t size;
  size_t capacity;

  void init(Game_Memory* memory, size_t count);

  T& operator[](size_t index)
  {
    return data[index];
  }
};

// @TODO change into enum
struct Game_Control
{
  union
  {
    struct Was
    {
      bool up_down;
      bool down_down;
      bool right_down;
      bool left_down;
      bool exit_down;
      bool enter_down;
    } was;
    bool was_down[6];
  };
  union
  {
    struct Is
    {
      bool up_down;
      bool down_down;
      bool right_down;
      bool left_down;
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

struct Text
{
  glm::vec2 pos;
  //@ TODO to array
  char* text;
  uint32_t text_len;
  glm::vec4 color_tint;
};


struct Sprite
{
  glm::vec2 pos;
  glm::vec2 size;
  glm::vec2 offset;
  glm::vec4 color_tint;
  int layer_order;
  int sprite_sheet_id;
};

struct Game_Context
{
  glm::vec2 dimension;
  float delta_time;
  Game_Control control;
  Game_Memory persist_memory;
  Game_Memory frame_memory;
};

struct Game_Output
{
  // @NOTE Immediate mode so dont draw the text is hidden ...
  // This pointer should be pointed at the right stuff by the game impl
  bool continue_running;
  Array<Text>* text_batch;
  Array<Sprite>* sprite_batch;
  Audio_Buffer audio_buffer;
};

struct The_Game
{
  struct Game_State* state;
  Game_Output* awake(Game_Context* context);
  Game_Output* update(Game_Context* context);
};

