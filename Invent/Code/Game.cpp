#include "Platform.h"

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



struct Game_State
{
  bool is_running;
  // Pong
  Entity_2D paddle_left;
  Entity_2D paddle_right;

  Entity_2D ball;
  int entity_count;

  int score_left;
  int score_right;

  // Main Menu

  void setup()
  {
    paddle_left.pos.x = 20;
    paddle_left.pos.y = 10;
    paddle_left.size.x = 20;
    paddle_left.size.y = 100;

    paddle_right.pos.x = 1080 - 40;
    paddle_right.pos.y = 10;
    paddle_right.size.x = 20;
    paddle_right.size.y = 100;

    ball.pos.x = 1080 / 2 - 8;
    ball.pos.y = 720 / 2 - 8;
    ball.size.x = 16;
    ball.size.y = 16;
    ball.vel.x = 500;
    //game_state->ball.vel.y = 300;

    entity_count = 3;

    score_left = 0;
    score_right = 0;
    is_running = true;
  }

  void update_main_menu(float delta_time, const Game_Input* game_input, const glm::vec2 window_dimension, Audio_Buffer* audio_buffer)
  {

  }

  void update_game(float delta_time, const Game_Input* game_input, const glm::vec2 window_dimension, Audio_Buffer* audio_buffer)
  {
    if (game_input->is.exit_down)
    {
      is_running = false;
    }

    // @TODO macro/ func to compress the "HOLD" check
    if (game_input->is.w_down && game_input->was.w_down) {
      paddle_left.acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
    }
    else if (game_input->is.s_down && game_input->was.s_down)
    {
      paddle_left.acc.y += 4000;
    }

    bool is_ai = true;
    if (!is_ai)
    {
      if (game_input->is.up_down && game_input->was.up_down) {
        paddle_right.acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
      }
      else if (game_input->is.down_down && game_input->was.down_down)
      {
        paddle_right.acc.y += 4000;
      }
    }
    else
    {
      if ((ball.pos.y + 100) > (paddle_right.pos.y + paddle_right.size.y))
      {
        paddle_right.acc.y += 4000;
      }
      else if ((ball.pos.y - 100) < (paddle_right.pos.y + paddle_right.size.y))
      {
        paddle_right.acc.y -= 4000; // code in the paddle movement speed arrgggg, boundry ????
      }
      else
      {
        paddle_right.acc.y = 0;
      }
    }

    // PADDLE MOTION
    paddle_left.acc -= paddle_left.vel * 10.f; // reduce acceleration impact when it already moving fast
    paddle_left.vel += paddle_left.acc * delta_time;
    paddle_left.pos += paddle_left.vel * delta_time * 2.0f;
    paddle_left.acc.y = 0;

    // @TODO ai used the same control no cheatiingg!!

    paddle_right.acc -= paddle_right.vel * 10.f; // reduce acceleration impact when it already moving fast
    paddle_right.vel += paddle_right.acc * delta_time;
    paddle_right.pos += paddle_right.vel * delta_time * 2.f;
    paddle_right.acc.y = 0;

    // Ball MOTION
    //paddle_right.acc -= paddle_right.vel * 10.f; // reduce acceleration impact when it already moving fast
    //paddle_right.vel += paddle_right.acc * deltaTime;
    ball.pos += ball.vel * delta_time;


    // COLLISION
    // @TODO compresss the 2 loop
    // top bottom clamp
    if ((paddle_left.pos.y + paddle_left.size.y) > window_dimension.y)
    {
      paddle_left.pos.y = window_dimension.y - paddle_left.size.y;
      paddle_left.vel.y = -paddle_left.vel.y;
    }
    if (paddle_left.pos.y < 0)
    {
      paddle_left.pos.y = 0;
      paddle_left.vel.y = -paddle_left.vel.y;
    }

    if ((paddle_right.pos.y + paddle_right.size.y) > window_dimension.y)
    {
      paddle_right.pos.y = window_dimension.y - paddle_right.size.y;
      paddle_right.vel.y = -paddle_right.vel.y;
    }
    if (paddle_right.pos.y < 0)
    {
      paddle_right.pos.y = 0;
      paddle_right.vel.y = -paddle_right.vel.y;
    }

    // Ball collosion
    if (ball.pos.y < 0)
    {
      ball.pos.y = 0;
      ball.vel.y = -ball.vel.y;
      audio_buffer->play_tick_sound = true;
    }
    if (ball.pos.y + ball.size.y > window_dimension.y)
    {
      ball.pos.y = window_dimension.y - ball.size.y;
      ball.vel.y = -ball.vel.y;
      audio_buffer->play_tick_sound = true;
    }
    // Point & Rest
    if (ball.pos.x < 0)
    {
      ball.pos.x = window_dimension.x / 2 - ball.size.x / 2;
      ball.pos.y = window_dimension.y / 2 - ball.size.y / 2;
      ball.vel.x = 0;
      ball.vel.y = 0;

      score_right += 1;
    }
    if (ball.pos.x + ball.size.x > window_dimension.x)
    {
      ball.pos.x = window_dimension.x / 2 - ball.size.x / 2;
      ball.pos.y = window_dimension.y / 2 - ball.size.y / 2;
      ball.vel.x = 0;
      ball.vel.y = 0;
      score_left += 1;
    }

    // @TODO sweep test 
    if (is_overlap(&ball, &paddle_left))
    {
      ball.vel.x = -ball.vel.x;
      ball.vel.y += paddle_left.vel.y * 0.5f;
      audio_buffer->play_tick_sound = true;
    }
    if (is_overlap(&ball, &paddle_right))
    {
      ball.vel.x = -ball.vel.x;
      ball.vel.y += paddle_right.vel.y * 0.5f;
      audio_buffer->play_tick_sound = true;
    }
  }
};
