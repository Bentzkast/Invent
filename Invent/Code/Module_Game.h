#pragma once
#include <glm/vec2.hpp>

enum Key
{
	EXIT,
	UP, DOWN, LEFT, RIGHT,
	SELECT,
	LMB, RMB,
	COUNT,
};

struct Game_Control
{
	bool is[Key::COUNT];
	bool was[Key::COUNT];
	glm::ivec2 mouse_pos;
};

class Module_Game
{
	bool is_running = false;
	glm::ivec2 window_dimension;
	float real_tick_elapsed = 0;
	Game_Control game_control;
public:
	glm::ivec2 get_window_dimension() const;
	void start_up();
	void start_frame();
	void shut_down();
	bool running() const;
};