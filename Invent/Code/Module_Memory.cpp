#include "Module_Memory.h"
#include "SDL_stdinc.h"
#include "SDL_assert.h"

bool Module_Memory::start_up()
{
	persist.init(1 * 1024 * 1024);
	frame.init(1 * 1024 * 1024);

  return true;
}

void Module_Memory::shut_down()
{
  persist.free();
  frame.free();
}


void Game_Memory::init(size_t size)
{
  base = (Uint8*)SDL_calloc(1, size);
  capacity = size;
  used = 0;
}

void* Game_Memory::allocate(size_t size)
{
  SDL_assert((used + size) <= capacity);
  void* result = base + used;
  used += size;
  return result;
}

void Game_Memory::clear()
{
  SDL_memset(base, 0, used);
  used = 0;
}

void Game_Memory::free()
{
  SDL_free(base);
}

void Module_Memory::clear_frame()
{
  frame.clear();
}
