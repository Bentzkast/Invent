
#include "Module_Memory.h"
#include "Module_Game.h"
#include "Module_SDL.h"

//template <typename T>
//struct Array {
//	T* data;
//	size_t size;
//	size_t capacity;
//
//	void init(Game_Memory* memory, size_t count);
//
//	T& operator[](size_t index)
//	{
//		return data[index];
//	}
//};

int main(int argc, char* argv[])
{
	Module_Memory memory{};
	memory.start_up();
	
	auto& game = memory.push_persist<Module_Game>();
	auto& sdl_module = memory.push_persist<Module_SDL>();
	
	game.start_up();
	if (sdl_module.start_up(game))
	{
		while (game.running())
		{
			memory.clear_frame();
			game.start_frame();

		}
	}
	game.shut_down();
	sdl_module.shut_down();
	memory.shut_down();
	return 0;
}