#include "Engine.h"

void array_push(Array<Text>* texts, glm::vec2 pos, char* text)
{
  SDL_assert((texts->size + 1) <= texts->capacity);
  int i = texts->size;
  texts->data[i].pos = pos;
  texts->data[i].text = text;
  texts->data[i].text_len = SDL_strlen(text);
  texts->data[i].color_tint = { 0.9f, 0.9f, 0.9f, 1.0f };
  texts->size++;
}

Sprite* sprite_push(Array<Sprite>* sprite, glm::vec2 pos, glm::vec2 size, glm::vec2 offset, glm::vec4 color)
{
  SDL_assert((sprite->size + 1) <= sprite->capacity);
  int i = sprite->size;
  sprite->data[i].pos = pos;
  sprite->data[i].size = size;
  sprite->data[i].offset = offset;
  sprite->data[i].color_tint = color;
  sprite->size++;

  return &sprite->data[i];
}

enum class Game_Mode
{
  MAIN_MENU,
  GAME_PLAY,
  COUNT
};

struct Unit
{
  glm::vec2 base_pos;
  Sprite* sprite;
};

struct Game_Play
{
  // Statics array..
  Array<Sprite> sprites;
  Array<Text> texts;

  Unit player;

  void awake(Game_Context* context, Game_Output* output)
  {
    sprites.init(&context->persist_memory, 200);
    texts.init(&context->persist_memory, 10);

    sprite_push(&sprites, { 10, 10 }, { 500, 500 }, { 8, 5 }, { 0.2f,0.4f, 0.2f,1.0f });

    //for (int y = 0; y < 10; y++)
    //{
    //  for (int x = 0; x < 10; x++)
    //  {
    //    sprite_push(&sprites, { x * 48, y * 48}, { 48, 48 }, { x, y }, { 0.9f,0.8f, 0.8f,1.0f });
    //  }
    //}
    player.base_pos = { 100, 100 };
    player.sprite = sprite_push(&sprites, { player.base_pos.x * 48, player.base_pos.y * 48 }, { 48, 48 }, { 27, 0 }, { 0.9f,0.8f, 0.8f,1.0f });
  }
  void start(Game_Context* context, Game_Output* output)
  {
    SDL_Log("ENTER GAME_PLAY");

  }
  Game_Mode update(Game_Context* context, Game_Output* output)
  {
    output->text_batch = &texts;
    output->sprite_batch = &sprites;
    Game_Control* control = &context->control;
    if (control->is.exit_down && !control->was.exit_down)
    {
      SDL_Log("MAIN_MENU");
      return Game_Mode::MAIN_MENU;
    }
    //SDL_Log("%f", context->delta_time);
    auto dt = context->delta_time;
    auto move_speed = 480 * dt;
    if (control->is.left_down && control->was.left_down)
    {
      player.base_pos.x -= move_speed;
    }
    if (control->is.right_down && control->was.right_down)
    {
      player.base_pos.x += move_speed;
    }
    if (control->is.down_down && control->was.down_down)
    {
      player.base_pos.y += move_speed;
    }
    if (control->is.up_down && control->was.up_down)
    {
      player.base_pos.y -= move_speed;
    }

    // Sync all sprites
    player.sprite->pos = player.base_pos;

    return Game_Mode::GAME_PLAY;
  }
};

struct Main_Menu
{
  enum Selection
  {
    PLAY,
    OPTIONS,
    EXIT,
    COUNT,
  };

  Selection current_selection;
  Array<Sprite> sprites;
  Array<Text> texts;
  Sprite* selection_sprite;

  void awake(Game_Context* context, Game_Output* game_output)
  {
    // Push Selection production pointer
    current_selection = Selection::PLAY;

    sprites.init(&context->persist_memory, 1);
    texts.init(&context->persist_memory, 10);


    // awake can be multiple time ... maybe also setup start
    selection_sprite = sprite_push(&sprites, { 0, 0 }, { 25, 25 }, { 8, 5 }, { 0.9f,0.9f, 0.9f,1.0f });
    constexpr int buffer_size = 20;


    char* title_buffer = context->persist_memory.push_array<char>(buffer_size);
    SDL_snprintf(title_buffer, buffer_size, "Micro Realm");
    array_push(&texts, { 20, 20 }, title_buffer);

    // @TODO font sizes...
    int padding = 50;
    int y_pos = 300;
    int x_pos = context->dimension.x / 4;

    char* play_buffer = context->persist_memory.push_array<char>(buffer_size);

    SDL_snprintf(play_buffer, buffer_size, "Play Game");

    array_push(&texts, { x_pos, y_pos }, play_buffer);
    y_pos += padding;

    char* options_buffer = context->persist_memory.push_array<char>(buffer_size);
    SDL_snprintf(options_buffer, buffer_size, "Options");
    array_push(&texts, { x_pos, y_pos }, options_buffer);

    y_pos += padding;

    char* exit_buffer = context->persist_memory.push_array<char>(buffer_size);
    SDL_snprintf(exit_buffer, buffer_size, "Exit Game");
    array_push(&texts, { x_pos, y_pos }, exit_buffer);


    game_output->audio_buffer.current_music = Music::MAIN_THEME;
  }

  void start(Game_Context* game_context, Game_Output* game_output)
  {
    SDL_Log("ENTER MAINMENU");

  }

  Game_Mode update(Game_Context* context, Game_Output* output)
  {
    output->text_batch = &texts;
    output->sprite_batch = &sprites;
    Game_Control* control = &context->control;

    if (control->is.down_down && !control->was.down_down)
    {
      current_selection = (Selection)((current_selection + 1) % Selection::COUNT);
      output->audio_buffer.play_bop_sound = true;
    }
    if (control->is.up_down && !control->was.up_down)
    {
      current_selection = (Selection)((current_selection - 1) < 0 ? Selection::COUNT - 1 : current_selection - 1);
      output->audio_buffer.play_bop_sound = true;
    }

    if (control->is.enter_down && !control->was.enter_down)
    {
      output->audio_buffer.play_bop_sound = true;

      switch (current_selection)
      {
      case Selection::PLAY: {
        SDL_Log("GAME_PLAY");
        return Game_Mode::GAME_PLAY;
      }break;
      case Selection::OPTIONS: {
        SDL_Log("NOT IMPLEMENTED");
      }break;
      case Selection::EXIT: {
        SDL_Log("Exiting");
        output->continue_running = false;
      }break;
      default:
        break;
      }
    }

    int padding = 50;
    int y_pos = 300;
    int x_pos = context->dimension.x / 4;

    selection_sprite->pos.x = context->dimension.x / 4 - 50;
    selection_sprite->pos.y = (current_selection)*padding + y_pos;
    
    return Game_Mode::MAIN_MENU;
  }
};

struct Game_State
{
  Main_Menu* main_menu;
  Game_Play* game_play;
  Game_Mode current_mode;
  Game_Mode next_mode;
};
Game_Output* The_Game::awake(Game_Context* context)
{
  Game_Output* output = context->frame_memory.push<Game_Output>();
  output->continue_running = true;

  state = context->persist_memory.push<Game_State>();

  state->main_menu = context->persist_memory.push<Main_Menu>();
  state->game_play = context->persist_memory.push<Game_Play>();

  state->next_mode = state->current_mode = Game_Mode::MAIN_MENU;
  state->main_menu->awake(context, output);
  state->game_play->awake(context, output);

  return output;
}

Game_Output* The_Game::update(Game_Context* context)
{
  Game_Output* output = context->frame_memory.push<Game_Output>();
  output->continue_running = true;

  bool changed_mode = state->current_mode != state->next_mode;
  state->current_mode = state->next_mode;

  switch (state->current_mode)
  {
  case Game_Mode::MAIN_MENU: {
    if (changed_mode) state->main_menu->start(context, output);
    state->next_mode = state->main_menu->update(context, output);
  }break;
  case Game_Mode::GAME_PLAY: {
    if (changed_mode) state->game_play->start(context, output);
    state->next_mode = state->game_play->update(context, output);
  }break;
  default:
    break;
  }

  return output;
}


//static bool is_overlap(const Entity_2D* entityA, const Entity_2D* entityB) // top left origin coordinate system
//{
//  if ((entityA->pos.x + entityA->size.x) < entityB->pos.x) return false;
//  if ((entityB->pos.x + entityB->size.x) < entityA->pos.x) return false;
//  if ((entityA->pos.y + entityA->size.y) < entityB->pos.y) return false;
//  if ((entityB->pos.y + entityB->size.y) < entityA->pos.y) return false;
//  return true;
//}
//static bool is_inside(const glm::vec2 vec, const Entity_2D* entity) // top left origin coordinate system
//{
//  if (vec.x < entity->pos.x) return false;
//  if ((entity->pos.x + entity->size.x) < vec.x) return false;
//  if (vec.y < entity->pos.y) return false;
//  if ((entity->pos.y + entity->size.y) < vec.y) return false;
//  return true;
//}

