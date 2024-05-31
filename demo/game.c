#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {750, 1000};

const char *BACKGROUND_PATH = "assets/background.png";
const char *USER_PATH = "assets/body.png";
const char *WALL_PATH = "assets/wall.jpeg";
const char *PLATFORM_PATH = "assets/platform.png";

const double BACKGROUND_CORNER = 150;
const double VERTICAL_OFFSET = 100;

// User constants
const double USER_MASS = 5;
const rgb_color_t USER_COLOR = (rgb_color_t){0, 0, 0};
const char *USER_INFO = "user";
const double USER_ROTATION = 0;
const vector_t USER_CENTER = {500, 60}; //(HERE JUST IN CASE NEED TO USE)
const double RADIUS = 15;
const size_t USER_NUM_POINTS = 20;
const double RESTING_SPEED = 200;
const double ACCEL = 100;
const double USER_JUMP_HEIGHT = 400;
const double GAP = 10;
const double VELOCITY_SCALE = 100;

// Wall constants
const vector_t WALL_WIDTH = {100, 0};
const size_t WALL_POINTS = 4;
const double WALL_MASS = INFINITY;
const double WALL_ELASTICITY = 0;
const size_t TEMP_LENGTH = 3;
const double NORMAL_SCALING = 1;
const double PLATFORM_SCALING = 5;
const double PLATFORM_HEIGHT = 100;
const vector_t PLATFORM_LENGTH = {0, 10};
const vector_t PLATFORM_WIDTH = {100, 0};
const double PLATFORM_ROTATION = M_PI/2;

const char *LEFT_WALL_INFO = "left_wall";
const char *RIGHT_WALL_INFO = "right_wall";
const char *PLATFORM_INFO = "platform";

// Game constants
const size_t NUM_LEVELS = 1;
const vector_t GRAVITY = {0, -980};
const size_t BODY_ASSETS = 3; // 2 walls and 1 platform

struct state {
  scene_t *scene;
  list_t *body_assets;
  bool is_jumping;
  asset_t *user_sprite;
  body_t *user_body;
  size_t user_health;
  size_t ghost_counter;
  double ghost_timer;
  double vertical_offset;
  bool game_over;
  bool collided;
};


list_t *make_user(double radius) {
  vector_t center = {MIN.x + radius + WALL_WIDTH.x, 
                    MIN.y + radius + PLATFORM_HEIGHT + PLATFORM_LENGTH.y};
  list_t *c = list_init(USER_NUM_POINTS, free);
  for (size_t i = 0; i < USER_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / USER_NUM_POINTS;
    vector_t *v = malloc(sizeof(*v));
    assert(v);
    *v = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(c, v);
  }
  return c;
}

/**
 * Sets the velocity of the user so that the user can jump from sticky walls
 */
void set_velocity(state_t *state, vector_t velocity){
  body_t *user = state -> user_body;
  body_set_velocity(user, velocity);
  vector_t center = body_get_centroid(state -> user_body);
  vector_t move = {velocity.x/VELOCITY_SCALE, velocity.y/VELOCITY_SCALE};
  body_set_centroid(user, vec_add(center, move));
}

/**
 * Generates the list of points for a Wall shape given the vector of the bottom left
 * corner
 *
 * @param corner a vector that contains the coordinates of the bottom left corner of
 * the wall
 * @param points an empty list to add the points to, the points are pointers to vectors
 */
void make_wall_points(vector_t corner, list_t *points){
  vector_t wall_length = {MIN.y, MAX.y};
  vector_t temp[] = {wall_length, vec_multiply(1, WALL_WIDTH), vec_negate(wall_length)};
  vector_t *v_1 = malloc(sizeof(*v_1));
  *v_1 = corner;
  list_add(points, v_1);
  assert(v_1);
  for (size_t i = 0; i < WALL_POINTS-1; i++){
    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(*(vector_t*)list_get(points, i), temp[i]);
    assert(v);
    list_add(points, v);
  }
}

void make_platform_points(vector_t corner, list_t *points){
  
  vector_t temp[] = {PLATFORM_LENGTH, PLATFORM_WIDTH, vec_negate(PLATFORM_LENGTH)};
  vector_t *v_1 = malloc(sizeof(*v_1));
  *v_1 = corner;
  list_add(points, v_1);
  assert(v_1);
  for (size_t i = 0; i < WALL_POINTS-1; i++){
    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(*(vector_t*)list_get(points, i), temp[i]);
    assert(v);
    list_add(points, v);
  }
}

list_t *make_wall(void *wall_info) {
  vector_t corner = VEC_ZERO;
  size_t cmp_left = strcmp(wall_info, LEFT_WALL_INFO);
  size_t cmp_right = strcmp(wall_info, RIGHT_WALL_INFO);
  size_t cmp_plat = strcmp(wall_info, PLATFORM_INFO);

  if (cmp_left == 0) {
    corner = MIN;
  } 
  if (cmp_right == 0) {
    corner = (vector_t){MAX.x - WALL_WIDTH.x, MIN.x};
  }
  if (cmp_plat == 0) {
    corner = (vector_t){MIN.x + WALL_WIDTH.x, PLATFORM_HEIGHT};
  }
  list_t *c = list_init(WALL_POINTS, free);
  if (cmp_left == 0 || cmp_right == 0){
    make_wall_points(corner, c);
  } else {
    make_platform_points(corner, c);
  }
  
  return c;
}

/**
 * Check conditions to see if game is over. Game is over if the user has no more health
 * (loss), the user falls off the map (loss)
 * or the user reaches the top of the map (win).
 *
 * @param state a pointer to a state object representing the current demo state
 */
bool game_over(state_t *state) {
  // ends the game, we will need in future but I wrote it by accident sorry ;)
  // vector_t user_pos = body_get_centroid(state->user_body);
  // if (user_pos.y + OUTER_RADIUS <= 0) {
  //   return true;
  // }
  return false;
}


void wall_init(state_t *state) {
  scene_t *scene = state -> scene;
  for (size_t i = 0; i < NUM_LEVELS; i++){
    list_t *left_points = make_wall((void *)LEFT_WALL_INFO);
    list_t *right_points = make_wall((void *)RIGHT_WALL_INFO);
    body_t *left_wall = body_init_with_info(left_points, WALL_MASS, 
                                            USER_COLOR, (void *)LEFT_WALL_INFO, 
                                            NULL);
    body_t *right_wall = body_init_with_info(right_points, INFINITY, 
                                            USER_COLOR, (void *)RIGHT_WALL_INFO, 
                                            NULL);
    scene_add_body(scene, left_wall);
    scene_add_body(scene, right_wall);
    asset_t *wall_asset_l = asset_make_image_with_body(WALL_PATH, left_wall, VERTICAL_OFFSET);
    asset_t *wall_asset_r = asset_make_image_with_body(WALL_PATH, right_wall, VERTICAL_OFFSET);
    list_add(state->body_assets, wall_asset_l);
    list_add(state->body_assets, wall_asset_r);
  }
  list_t *platform_points = make_wall((void *)PLATFORM_INFO);
  body_t *platform = body_init_with_info(platform_points, INFINITY, 
                                            USER_COLOR, (void *)PLATFORM_INFO, 
                                            NULL);
  scene_add_body(scene, platform);
  create_collision(scene, platform, state -> user_body, physics_collision_handler, (char*)"v_0", WALL_ELASTICITY);
  asset_t *wall_asset_platform = asset_make_image_with_body(PLATFORM_PATH, platform, VERTICAL_OFFSET);
  list_add(state->body_assets, wall_asset_platform);
}

/**
 * @return the boolean value of the comparison between @param v1 and @param v2, returns true
 * if they are equal and false if they are not
 */
bool vec_cmp(vector_t v1, vector_t v2){
  return((v1.x == v2.x) && (v1.y == v2.y));
}


/**
 * Check whether two bodies are colliding and applies a sticky collision between them
 * and to be called every tick
 *
 * @param body1
 * @param body2 the two bodies two check for a collision between, and if they are colliding
 * sets both velocities to be 0
 */
void sticky_collision(state_t *state, body_t *body1, body_t *body2){
  vector_t v1 = body_get_velocity(body1);
  vector_t v2 = body_get_velocity(body2);
  state -> collided = find_collision(body1, body2).collided;
  bool velocity_zero = (vec_cmp(v1, VEC_ZERO) && vec_cmp(v2, VEC_ZERO));
  if (state -> collided && !velocity_zero){
    if (state->is_jumping) {
    body_set_velocity(body1, VEC_ZERO);
    body_set_velocity(body2, VEC_ZERO);
    state->is_jumping = false;
    } else if (strcmp(body_get_info(body2), PLATFORM_INFO) == 0) {
      body_set_velocity(body1, (vector_t) {v1.x, 0});
    }
  }
}

/**
 * Move player on display screen based on key pressed.
 *
 * @param key the character of the key pressed
 * @param type event type connected to key
 * @param held_time double value representing the amount of time the key is held
 * down
 * @param state the state representing the current demo
 */
void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  body_t *user = state->user_body;
  vector_t cur_v = body_get_velocity(user);
  double new_vx = cur_v.x;
  double new_vy = cur_v.y;

  if (type == KEY_PRESSED) {
      switch (key) {
      case LEFT_ARROW: {
        if (!state->is_jumping) {
          new_vx = -1 * (RESTING_SPEED + ACCEL * held_time);
        }
        break;
      }
      case RIGHT_ARROW: {
        if (!state->is_jumping) {
          new_vx = RESTING_SPEED + ACCEL * held_time;
        }
        break;
      }
      case UP_ARROW: {
        if (!state->is_jumping) {
          new_vy = USER_JUMP_HEIGHT;
          state->is_jumping = true;
        }
        break;
      }
    }
  }
  body_set_velocity(user, (vector_t) {new_vx, new_vy});
}

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  asset_cache_init();
  state_t *state = malloc(sizeof(state_t));
  assert(state);

  // intialize scene and user
  state->scene = scene_init();
  state->body_assets = list_init(BODY_ASSETS, (free_func_t)asset_destroy);
  list_t *points = make_user(RADIUS);
  state->user_body =
      body_init_with_info(points, USER_MASS, USER_COLOR, (void *)USER_INFO, NULL);
  body_t* body = state->user_body;

  vector_t initial_velocity = {20, 20};
  set_velocity(state, initial_velocity);

  // Create and save the asset for the background image
  SDL_Rect background_box = {.x = MIN.x, .y = MIN.y, .w = MAX.x, .h = MAX.y};
  asset_t *background_asset = asset_make_image(BACKGROUND_PATH, background_box);
  list_add(state->body_assets, background_asset);

  // Create and save the asset for the user image
  asset_t *user_asset = asset_make_image_with_body(USER_PATH, body, state->vertical_offset);
  list_add(state->body_assets, user_asset);

  wall_init(state);

  state->game_over = false;
  state->collided = false;
  state->vertical_offset = 0;

  // initalize key handler
  sdl_on_key((key_handler_t)on_key);
  state->is_jumping = false;

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  body_t *user = state->user_body;
  scene_t *scene = state->scene;
  scene_tick(scene, dt);
  body_tick(user, dt);
  sdl_clear();

  vector_t player_pos = body_get_centroid(user);
  state->vertical_offset = player_pos.y - VERTICAL_OFFSET;

  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i), state->vertical_offset);
  }

  sdl_show(state->vertical_offset);
  for (size_t i = 0; i < scene_bodies(scene); i++){
    body_t *wall = scene_get_body(scene, i);
    sticky_collision(state, user, wall);
  }

  // include gravity
  body_add_force(state -> user_body, GRAVITY);

  return game_over(state);
}


void emscripten_free(state_t *state) {
  TTF_Quit();
  scene_free(state->scene);
  list_free(state->body_assets);
  body_free(state->user_body);
  asset_cache_destroy();
  free(state);
}