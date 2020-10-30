#include "Engine.h"

static void array_push(Array<Text>* texts, glm::vec2 pos, char* text)
{
  SDL_assert((texts->size + 1) <= texts->capacity);
  int i = texts->size;
  texts->data[i].pos = pos;
  texts->data[i].text = text;
  texts->data[i].text_len = SDL_strlen(text);
  texts->data[i].color_tint = { 0.9f, 0.9f, 0.9f, 1.0f };
  texts->size++;
}

static Sprite* sprite_push(Array<Sprite>* sprite, glm::vec2 pos, glm::vec2 size, glm::vec2 offset, glm::vec4 color)
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

struct Formation
{
  glm::vec2 pos;
  glm::vec2 vel;
  glm::ivec2 dimension;
  glm::vec2 direction;
  int filled_slot;
};

struct Unit
{
  glm::vec2 base_pos;
  Sprite* sprite;
  Formation* formation;
};

struct Game_Play
{
  // Statics array..
  Array<Sprite> sprites;
  Array<Text> texts;

  // @TODO start army formation
  // @TODO amry vs army
  Sprite* cursor;
  Array<Unit> army_one;
  Array<Unit> grasses;

  Formation formation;
  float rot;
  glm::vec2 target_pos;

  void awake(Game_Context* context, Game_Output* output)
  {
    sprites.init(&context->persist_memory, 200);
    texts.init(&context->persist_memory, 10);

    army_one.init(&context->persist_memory, 10);
    grasses.init(&context->persist_memory, 20);

    cursor = sprite_push(&sprites, { 0, 0 }, { 48, 48 }, { 36, 10 }, { 0.9f,0.8f, 0.8f,1.0f });
    cursor->layer_order = context->dimension.y;
    target_pos = { 500 , 500 };

    for (size_t i = 0; i < 10; i++)
    {
      grasses[i].base_pos = { rand() % (int)context->dimension.x, rand() % (int)context->dimension.y };
      grasses[i].sprite = sprite_push(&sprites, grasses[i].base_pos, { 48, 48 }, { 0, 2 }, { 0.9f,0.8f, 0.8f,1.0f });
      grasses[i].sprite->layer_order = grasses[i].base_pos.y;
      grasses.size++;
    }
    for (int y = 0; y < 10; y++)
    {
      army_one[y].base_pos = { 0, 0 };
      army_one[y].sprite = sprite_push(&sprites, army_one[y].base_pos, { 48, 48 }, { 27, 0 }, { 0.9f,0.8f, 0.8f,1.0f });
      // TODO max formation size???
      army_one[y].formation = &formation;
      army_one.size++;
    }

    rot = 0;

    formation.pos = { 500, 500 };
    formation.vel = { 0, 0 };
    formation.direction.x = glm::cos(glm::radians(rot));
    formation.direction.y = -glm::cos(glm::radians(rot));
    formation.dimension = { 5, 2 };
    formation.filled_slot = 0;
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

    cursor->pos = control->mouse_pos;
    //SDL_Log("%f", context->delta_time);
    auto dt = context->delta_time;
    auto move_speed = 240 * dt;


    if (control->is.left_mouse && !control->was.left_mouse)
    {
      target_pos = cursor->pos;
    }
    rot += dt * 200;
    auto dir = glm::normalize(target_pos - formation.pos);
    //if (glm::length(target_pos - formation.pos) > 10.0f)
    //{
    //  formation.direction = dir; //hmmm
    //  formation.pos += move_speed * formation.direction;
    //}
    formation.direction.x = glm::cos(glm::radians(rot));
    formation.direction.y = -glm::sin(glm::radians(rot));
    //formation.pos += formation.vel * dt;
    // clear formation slots
    formation.filled_slot = 0;


    // fill in formation slots
    // @TODO readjust when taking sub strenght
    // @TODO stutter problem??
    for (size_t i = 0; i < army_one.size; i++)
    {
      auto formation = army_one[i].formation;
      auto perp = glm::normalize(perpendicular_clockwise(formation->direction));
      auto back = glm::normalize(-formation->direction);
       
      int pos_in_row = formation->filled_slot % formation->dimension.x;
      int pos_in_col = formation->filled_slot / formation->dimension.x;
      auto pos = formation->pos - perp * 48.0f * (float)formation->dimension.x / 2.0f + perp * 24.0f;
      army_one[i].base_pos = pos + perp * 48.0f * (float)pos_in_row + back * 48.0f * (float)pos_in_col;
      formation->filled_slot++;
    }

    // Sync all sprites
    for (int i = 0; i < army_one.size; i++)
    {
      army_one[i].sprite->pos.y = army_one[i].base_pos.y - army_one[i].sprite->size.y;
      army_one[i].sprite->pos.x = army_one[i].base_pos.x - army_one[i].sprite->size.x / 2.0 ;
      army_one[i].sprite->layer_order = army_one[i].sprite->pos.y;
    }

    for (int i = 0; i < army_one.size; i++)
    {
      grasses[i].sprite->pos.y = grasses[i].base_pos.y - grasses[i].sprite->size.y;
      grasses[i].sprite->pos.x = grasses[i].base_pos.x - grasses[i].sprite->size.x / 2.0f;
      grasses[i].sprite->layer_order = grasses[i].sprite->pos.y;
    }

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
  Sprite* cursor;


  void awake(Game_Context* context, Game_Output* game_output)
  {
    // Push Selection production pointer
    current_selection = Selection::PLAY;

    sprites.init(&context->persist_memory, 2);
    texts.init(&context->persist_memory, 10);


    // awake can be multiple time ... maybe also setup start
    selection_sprite = sprite_push(&sprites, { 0, 0 }, { 25, 25 }, { 8, 5 }, { 0.9f,0.9f, 0.9f,1.0f });
    cursor = sprite_push(&sprites, { 0, 0 }, { 48, 48 }, { 36, 10 }, { 0.9f,0.8f, 0.8f,1.0f });
    cursor->layer_order = context->dimension.y;

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
    cursor->pos = control->mouse_pos;

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

