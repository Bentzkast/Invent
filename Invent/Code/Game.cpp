#include "Engine.h"



struct Entity_2D
{
  enum Type
  {
    STATIC_COLOR,
    STATIC_SPRITE,
    COUNT,
  };

  Type type;
  glm::vec2 pos; // top left origin
  glm::vec2 size;
  union {
    glm::vec4 color;
    Sprite sprite;
  };
};
static bool is_overlap(const Entity_2D* entityA, const Entity_2D* entityB) // top left origin coordinate system
{
  if ((entityA->pos.x + entityA->size.x) < entityB->pos.x) return false;
  if ((entityB->pos.x + entityB->size.x) < entityA->pos.x) return false;
  if ((entityA->pos.y + entityA->size.y) < entityB->pos.y) return false;
  if ((entityB->pos.y + entityB->size.y) < entityA->pos.y) return false;
  return true;
}
static bool is_inside(const glm::vec2 vec, const Entity_2D* entity) // top left origin coordinate system
{
  if (vec.x < entity->pos.x) return false;
  if ((entity->pos.x + entity->size.x) < vec.x) return false;
  if (vec.y < entity->pos.y) return false;
  if ((entity->pos.y + entity->size.y) < vec.y) return false;
  return true;
}

enum class Game_Mode
{
  MENU,
  PONG
};

static int push_static_color_2d(Array<Entity_2D>& entities, glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
  SDL_assert((entities.size + 1) <= entities.capacity);
  int index = entities.size;
  Entity_2D* e = &entities[index];
  e->type = Entity_2D::STATIC_COLOR;
  e->pos = pos;
  e->size = size;
  e->color = color;
  entities.size++;
  return index;
}

static int push_static_sprite_2d(Array<Entity_2D>& entities, glm::vec2 pos, glm::vec2 size, Sprite sprite)
{
  SDL_assert((entities.size + 1) <= entities.capacity);
  int index = entities.size;
  Entity_2D* e = &entities[index];
  e->type = Entity_2D::STATIC_SPRITE;
  e->pos = pos;
  e->size = size;
  e->sprite = sprite;
  entities.size++;
  return index;
}

struct Game_Play
{
  Array<Entity_2D> entities;

  int world_offset_x;
  int world_offset_y;

  void setup(Memory_Chunk* main_memory)
  {
    entities.Init(main_memory, 100);
    Sprite temp = { 0 };
    
    for (int y = 0; y < 10; y++)
    {
      for (int x = 0; x < 10; x++)
      {
        temp.x_shift = x;
        temp.y_shift = y;
        push_static_sprite_2d(entities, { x * 48 , y * 48 }, { 48, 48 }, temp);
      }
    }
    world_offset_x = 0;
    world_offset_y = 0;
  }

  Game_Mode update(Memory_Chunk* frame_memory,
    float delta_time, const Game_Input* game_input,
    Audio_Buffer* audio_buffer,
    Game_Screen* game_screen)
  {
    if (game_input->is.exit_down)
    {
      return Game_Mode::MENU;
    }
    if (game_input->is.up_down && !game_input->was.up_down)
    {
      if (world_offset_y > 0)
      {
        world_offset_y--;
      }
    }
    if (game_input->is.down_down && !game_input->was.down_down)
    {
      if (world_offset_y < 22 - 10)
      {
        world_offset_y++;
      }
    }
    if (game_input->is.left_down && !game_input->was.left_down)
    {
      if (world_offset_x > 0)
      {
        world_offset_x--;
      }
    }
    if (game_input->is.right_down && !game_input->was.right_down)
    {
      if (world_offset_x < 48 - 10)
      {
        world_offset_x++;
      }
    }


    for (int y = 0; y < 10; y++)
    {
      for (int x = 0; x < 10; x++)
      {
        Entity_2D* e = &entities[(size_t)(y * 10 + x)];
        e->sprite.x_shift = x + world_offset_x;
        e->sprite.y_shift = y + world_offset_y;
      }
    }

    // Ui stuff
    char* score_left_buffer = (char*)frame_memory->allocate(sizeof(char*) * 20);
    SDL_snprintf(score_left_buffer, 20, "Status Panel");

    game_screen->push_screen_texts({ game_screen->dimension.x - 400, 50 }, score_left_buffer);

    return Game_Mode::PONG;
  }
};


struct Game_MainMenu
{
  enum Selection
  {
    PLAY,
    OPTIONS,
    EXIT,
    COUNT,
  };

  Array<Entity_2D> entities;

  Selection current_selection;
  int selection_object_id;

  void setup(Memory_Chunk* main_memory, Audio_Buffer* audio_buffer)
  {
    entities.Init(main_memory, 1);
    // Push Selection production pointer
    current_selection = Selection::PLAY;
    selection_object_id = push_static_color_2d(entities, { 0, 0 }, { 25, 25 }, { .9f, .9f, .9f, 1.0f });
    audio_buffer->current_music = Music::MAIN_THEME;
  }

  Game_Mode update(
    bool* is_running, 
    Memory_Chunk* frame_memory,
    const Game_Input* game_input, 
    Game_Screen* game_screen,
    Audio_Buffer* audio_buffer)
  {
    if (game_input->is.down_down && !game_input->was.down_down)
    {
      current_selection = (Selection)((current_selection + 1) % Selection::COUNT);
      audio_buffer->play_bop_sound = true;
    }
    if (game_input->is.up_down && !game_input->was.up_down)
    {
      current_selection = (Selection)((current_selection - 1) < 0 ?  Selection::COUNT - 1: current_selection - 1);
      audio_buffer->play_bop_sound = true;
    }

    if (game_input->is.enter_down && !game_input->was.enter_down)
    {
      switch (current_selection)
      {
        case Selection::PLAY: {
          return Game_Mode::PONG;
        }break;
        case Selection::OPTIONS: {
          SDL_Log("NOT IMPLEMENTED");
        }break;
        case Selection::EXIT: {
          *is_running = false;
        }break;
        default:
          break;
      }
    }

    size_t buffer_size = 20;

    // @TODO font sizes...
    char* title_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(title_buffer, buffer_size, "Micro Realm");
    game_screen->push_screen_texts({ 10, 10 }, title_buffer);

    int padding = 50;
    int y_pos = 300;
    int x_pos = game_screen->dimension.x / 4;

    entities[selection_object_id].pos.x = game_screen->dimension.x / 4 - 50;
    entities[selection_object_id].pos.y = (current_selection)*padding + y_pos; // title size

    char* play_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(play_buffer, buffer_size, "Play Game");
    game_screen->push_screen_texts({ x_pos, y_pos }, play_buffer);
    y_pos += padding;
    
    char* options_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(options_buffer, buffer_size, "Options");
    game_screen->push_screen_texts({ x_pos, y_pos }, options_buffer);
    y_pos += padding;

    char* exit_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(exit_buffer, buffer_size, "Exit Game");
    game_screen->push_screen_texts({ x_pos, y_pos }, exit_buffer);
    y_pos += padding;

    return Game_Mode::MENU;
  }
};

struct Game_State
{
  bool is_running;
  Game_Mode current_mode;
  Game_Mode next_mode;

  Game_Play game_play;
  Game_MainMenu main_menu;
  
  Array<Entity_2D>* entities;

  void setup(Memory_Chunk* main_memory, Audio_Buffer* audio_buffer)
  {
    next_mode = current_mode = Game_Mode::MENU;
    main_menu.setup(main_memory, audio_buffer);
    is_running = true;
  }

  void update(Memory_Chunk* main_memory, Memory_Chunk* frame_memory,
    float delta_time, const Game_Input* game_input,
    Audio_Buffer* audio_buffer,
    Game_Screen* game_screen)
  {
    bool changed_mode = current_mode != next_mode;
    current_mode = next_mode;

    switch (current_mode)
    {
    case Game_Mode::MENU: {
      if (changed_mode) main_menu.setup(main_memory, audio_buffer);

      entities = &main_menu.entities;
       next_mode = main_menu.update(&is_running, frame_memory, game_input, game_screen, audio_buffer);
    }break;
    case Game_Mode::PONG: {
      if (changed_mode) game_play.setup(main_memory);

      entities = &game_play.entities;
      next_mode = game_play.update(frame_memory, delta_time, game_input, audio_buffer, game_screen);
    }break;
    default:
      break;
    }
  }
};
