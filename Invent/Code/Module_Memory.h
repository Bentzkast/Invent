#pragma once
#include "SDL_stdinc.h"

class Game_Memory
{
  size_t capacity = 0;
  size_t used = 0;
  uint8_t* base = nullptr;
public:
  void init(size_t size);
  void* allocate(size_t size);
  void clear();
  void free();
};

class Module_Memory
{
  Game_Memory persist{};
  Game_Memory frame{};
public:
	bool start_up();
	void shut_down();

  template<typename T>
  inline T& push_persist(size_t count = 1)
  {
    return *((T*)persist.allocate(sizeof(T) * count));
  }

  template<typename T>
  inline T* push_frame(size_t count = 1)
  {
    return (T*)frame.allocate(sizeof(T) * count);
  }

  void clear_frame();
};

