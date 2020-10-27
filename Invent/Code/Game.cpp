#include "Platform.h"

enum class Entity_Type
{
  Simple_2D
};

struct Entity_2D
{
  glm::vec2 pos; // top left origin
  glm::vec2 size;
  glm::vec2 vel;
  glm::vec2 acc;
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


static int push_simple_2d(Array<Entity_2D>& entities, glm::vec2 pos, glm::vec2 size, glm::vec2 vel)
{
  SDL_assert((entities.size + 1) <= entities.capacity);
  int index = entities.size;
  entities[entities.size].pos = pos;
  entities[entities.size].size = size;
  entities[entities.size].vel = vel;
  entities.size++;
  return index;
}

struct Game_Play
{
  Array<Entity_2D> entities;

  int score_left;
  int score_right;
  int match_point;

  int paddle_left_index = 0;
  int paddle_right_index = 0;
  int ball_index = 0;

  Entity_2D* paddle_left;
  Entity_2D* paddle_right;
  Entity_2D* ball;

  void setup(Memory_Chunk* main_memory)
  {
    entities.Init(main_memory, 3);

    paddle_left_index = push_simple_2d(entities, { 20,20 }, { 20, 100 }, { 0 ,0 });
    paddle_right_index = push_simple_2d(entities, { 1080 - 40,10 }, { 20, 100 }, { 0 ,0 });
    ball_index = push_simple_2d(entities, { 1080 / 2 - 8, 720 / 2 - 8 }, { 16, 16}, { 500 ,0 });

    paddle_left = &entities[paddle_left_index];
    paddle_right = &entities[paddle_right_index];
    ball = &entities[ball_index];

    match_point = 5;
    score_left = 0;
    score_right = 0;
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

    // @TODO macro/ func to compress the "HOLD" check
    if (game_input->is.w_down && game_input->was.w_down) {
      paddle_left->acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
    }
    else if (game_input->is.s_down && game_input->was.s_down)
    {
      paddle_left->acc.y += 4000;
    }

    bool is_ai = true;
    if (!is_ai)
    {
      if (game_input->is.up_down && game_input->was.up_down) {
        paddle_right->acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
      }
      else if (game_input->is.down_down && game_input->was.down_down)
      {
        paddle_right->acc.y += 4000;
      }
    }
    else
    {
      if ((ball->pos.y + 100) > (paddle_right->pos.y + paddle_right->size.y))
      {
        paddle_right->acc.y += 4000;
      }
      else if ((ball->pos.y - 100) < (paddle_right->pos.y + paddle_right->size.y))
      {
        paddle_right->acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
      }
      else
      {
        paddle_right->acc.y = 0;
      }
    }

    // PADDLE MOTION
    paddle_left->acc -= paddle_left->vel * 10.f; // reduce acceleration impact when it already moving fast
    paddle_left->vel += paddle_left->acc * delta_time;
    paddle_left->pos += paddle_left->vel * delta_time * 2.0f;
    paddle_left->acc.y = 0;

    // @TODO ai used the same control no cheatiingg!!

    paddle_right->acc -= paddle_right->vel * 10.f; // reduce acceleration impact when it already moving fast
    paddle_right->vel += paddle_right->acc * delta_time;
    paddle_right->pos += paddle_right->vel * delta_time * 2.f;
    paddle_right->acc.y = 0;

    // Ball MOTION
    //paddle_right.acc -= paddle_right.vel * 10.f; // reduce acceleration impact when it already moving fast
      //paddle_right.vel += paddle_right.acc * delta_time;
    ball->pos += ball->vel * delta_time;


    // COLLISION
    // @TODO compresss the 2 loop
    // top bottom clamp
    if ((paddle_left->pos.y + paddle_left->size.y) > game_screen->dimension.y)
    {
      ;
      paddle_left->pos.y = game_screen->dimension.y - paddle_left->size.y;
      paddle_left->vel.y = -paddle_left->vel.y;
    }
    if (paddle_left->pos.y < 0)
    {
      paddle_left->pos.y = 0;
      paddle_left->vel.y = -paddle_left->vel.y;
    }

    if ((paddle_right->pos.y + paddle_right->size.y) > game_screen->dimension.y)
    {
      paddle_right->pos.y = game_screen->dimension.y - paddle_right->size.y;
      paddle_right->vel.y = -paddle_right->vel.y;
    }
    if (paddle_right->pos.y < 0)
    {
      paddle_right->pos.y = 0;
      paddle_right->vel.y = -paddle_right->vel.y;
    }

    // Ball collosion
    if (ball->pos.y < 0)
    {
      ball->pos.y = 0;
      ball->vel.y = -ball->vel.y;
      audio_buffer->play_tick_sound = true;
    }
    if (ball->pos.y + ball->size.y > game_screen->dimension.y)
    {
      ball->pos.y = game_screen->dimension.y - ball->size.y;
      ball->vel.y = -ball->vel.y;
      audio_buffer->play_tick_sound = true;
    }
    // Point & Rest
    if (ball->pos.x < 0)
    {
      ball->pos.x = game_screen->dimension.x / 2 - ball->size.x / 2;
      ball->pos.y = game_screen->dimension.y / 2 - ball->size.y / 2;
      ball->vel.x = -200;
      ball->vel.y = 0;

      score_right += 1;
    }
    if (ball->pos.x + ball->size.x > game_screen->dimension.x)
    {
      ball->pos.x = game_screen->dimension.x / 2 - ball->size.x / 2;
      ball->pos.y = game_screen->dimension.y / 2 - ball->size.y / 2;
      ball->vel.x = 0;
      ball->vel.y = 200;

      score_left += 1;
    }

    // @TODO sweep test 
    if (is_overlap(ball, paddle_left))
    {
      ball->vel.x = -ball->vel.x;
      ball->vel.y += paddle_left->vel.y * 0.5f;
      audio_buffer->play_tick_sound = true;
    }
    if (is_overlap(ball, paddle_right))
    {
      ball->vel.x = -ball->vel.x;
      ball->vel.y += paddle_right->vel.y * 0.5f;
      audio_buffer->play_tick_sound = true;
    }

    // Ui stuff
    char* score_left_buffer = (char*)frame_memory->allocate(sizeof(char*) * 20);
    SDL_snprintf(score_left_buffer, 20, "%d\0", score_left);

    game_screen->push_screen_texts({ 100, 50 }, score_left_buffer);


    char* score_right_buffer = (char*)frame_memory->allocate(sizeof(char*) * 20);
    SDL_snprintf(score_right_buffer, 20, "%d\0", score_right);

    game_screen->push_screen_texts({ game_screen->dimension.x - 100, 50 }, score_right_buffer);


    return Game_Mode::PONG;
  }
};

struct Game_MainMenu
{
  Array<Entity_2D> entities;

  int selection_index = 0;
  int selection_pointer_id;

  void setup(Memory_Chunk* main_memory, Audio_Buffer* audio_buffer)
  {
    entities.Init(main_memory, 1);
    selection_pointer_id = 0;
    // Push Selection production pointer
    push_simple_2d(entities, { 0, 0 }, { 25, 25 }, { 0, 0 });
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
      selection_index = (selection_index + 1) % 2;
      audio_buffer->play_bop_sound = true;
    }
    if (game_input->is.up_down && !game_input->was.up_down)
    {
      selection_index = (selection_index - 1) < 0 ? 1 : selection_index - 1;
      audio_buffer->play_bop_sound = true;
    }
    if (game_input->is.enter_down && selection_index == 1)
    {
      *is_running = false;
    }
    if (game_input->is.enter_down && selection_index == 0)
    {
      return Game_Mode::PONG;
    }

    int padding = 50;
    entities[selection_pointer_id].pos.x = game_screen->dimension.x / 4 - 50;
    entities[selection_pointer_id].pos.y = (selection_index + 1) * padding + 100; // title size

    // @TODO better margin for title
    // @TODO font sizes...
    int y_pos = padding;
    int x_pos = game_screen->dimension.x / 4;

    size_t buffer_size = 20;

    char* title_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(title_buffer, buffer_size, "PONG\0");
    game_screen->push_screen_texts({ x_pos, y_pos }, title_buffer);
    y_pos += padding * 2;

    char* score_left_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(score_left_buffer, buffer_size, "Play Game\0");
    game_screen->push_screen_texts({ x_pos, y_pos }, score_left_buffer);
    y_pos += padding;

    char* score_right_buffer = (char*)frame_memory->allocate(sizeof(char*) * buffer_size);
    SDL_snprintf(score_right_buffer, buffer_size, "Exit Game\0");
    game_screen->push_screen_texts({ x_pos, y_pos }, score_right_buffer);
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
