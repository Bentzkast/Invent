#pragma once
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    } was;
    bool was_down[5];
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
    } is;
    bool is_down[5];
  };
};

struct Audio_Buffer
{
  bool play_tick_sound;
};
