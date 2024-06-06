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
#include "vector.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {750, 1000};

// File paths
const char *BACKGROUND_PATH = "assets/background.png";
const char *USER_PATH = "assets/body.png";
const char *WALL_PATH = "assets/wall.png";
const char *PLATFORM_PATH = "assets/platform.png";
const char *JUMP_POWERUP_PATH = "assets/jump_powerup.png";
const char *HEALTH_POWERUP_PATH = "assets/health_powerup.png";
const char *FULL_HEALTH_BAR_PATH = "assets/health_bar_3.png";
const char *HEALTH_BAR_2_PATH = "assets/health_bar_2.png";
const char *HEALTH_BAR_1_PATH = "assets/health_bar_1.png";

// User constants
const double USER_MASS = 5;
const double USER_ROTATION = 0;
const size_t USER_NUM_POINTS = 20;
const double USER_JUMP_HEIGHT = 400;
const rgb_color_t USER_COLOR = (rgb_color_t){0, 0, 0};
const double RADIUS = 25;
const double RESTING_SPEED = 200;
const double VELOCITY_SCALE = 100;
const double ACCEL = 100;
const size_t JUMP_BUFFER = 30; // how many pixels away from wall can user jump
const double GAP = 10;
const size_t FULL_HEALTH = 3;

// Wall constants
const vector_t WALL_WIDTH = {100, 0};
const size_t WALL_POINTS = 4;
const double WALL_MASS = INFINITY;
const double WALL_ELASTICITY = 0;
const size_t TEMP_LENGTH = 3;
const double NORMAL_SCALING = 1;
const double PLATFORM_SCALING = 5;
const double PLATFORM_HEIGHT = 100;
const vector_t PLATFORM_LENGTH = {0, 15};
const vector_t PLATFORM_WIDTH = {110, 0};
const double PLATFORM_ROTATION = M_PI/2;
const double PLATFORM_FRICTION = .85;

// health bar location
const vector_t HEALTH_BAR_MIN = {15, 15};
const vector_t HEALTH_BAR_MAX = {90, 30};
SDL_Rect HEALTH_BAR_BOX = {.x = HEALTH_BAR_MIN.x, .y = HEALTH_BAR_MIN.y, 
                           .w = HEALTH_BAR_MAX.x, .h = HEALTH_BAR_MAX.y};

// powerup constants
const size_t POWERUP_LOC = 50; // radius from tower center where powerups generated
const size_t JUMP_POWERUP_LOC = (size_t) 2 * (MAX.y / 3);
const size_t HEALTH_POWERUP_LOC = (size_t) (MAX.y / 3);
const double jump_powerup_time = 7; // how long jump powerup lasts
const double POWERUP_LENGTH = 18;
const double POWERUP_MASS = .0001;
const double POWERUP_ELASTICITY = 1;

// Game constants
const size_t NUM_LEVELS = 1;
const vector_t GRAVITY = {0, -1000};
const size_t BODY_ASSETS = 3; // total assets, 2 walls and 1 platform
const double BACKGROUND_CORNER = 150;
const double VERTICAL_OFFSET = 100;

typedef enum { USER, LEFT_WALL, RIGHT_WALL, PLATFORM, JUMP_POWER, HEALTH_POWER, NONE } body_type_t;

struct state {
  scene_t *scene;
  list_t *body_assets;
  asset_t *user_sprite;
  body_t *user;
  
  size_t user_health;
  asset_t *health_bar;

  size_t ghost_counter;
  double ghost_timer;
  double vertical_offset;
  bool game_over;
  
  bool jumping;
  size_t can_jump;
  body_t *collided_obj;
  
  bool jump_powerup;
  double jump_powerup_time;

  size_t jump_powerup_index;
  size_t health_powerup_index;
};

body_type_t get_type(body_t *body) {
  if (body == NULL) {
    return NONE;
  }
  return *(body_type_t *)body_get_info(body);
}

body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  *info = type;
  return info;
}

// /**
//  * Sets the velocity of the user so that the user can jump from sticky walls
//  * 
//  * @param state state object representing the current demo state
//  * @param velocity velocity to set the user to 
//  */
// void set_velocity(state_t *state, vector_t velocity){
//   body_t *user = state -> user;
//   body_set_velocity(user, velocity);
//   vector_t center = body_get_centroid(state -> user);
//   vector_t move = {velocity.x/VELOCITY_SCALE, velocity.y/VELOCITY_SCALE};
//   body_set_centroid(user, vec_add(center, move));
// }

/**
 * Creates user shape.
 * 
 * @return list_t containing the points of the shape
*/
list_t *make_user() {
  vector_t center = {MIN.x + RADIUS + WALL_WIDTH.x, 
                    MIN.y + RADIUS + PLATFORM_HEIGHT + PLATFORM_LENGTH.y};
  list_t *c = list_init(USER_NUM_POINTS, free);
  for (size_t i = 0; i < USER_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / USER_NUM_POINTS;
    vector_t *v = malloc(sizeof(*v));
    assert(v);
    *v = (vector_t){center.x + RADIUS * cos(angle),
                    center.y + RADIUS * sin(angle)};
    list_add(c, v);
  }
  return c;
}

/**
 * Generates the list of points for a Wall shape given the vector of the bottom left
 * corner
 *
 * @param corner a vector that contains the coordinates of the bottom left corner of
 * the wall
 * @param points an empty list to add the points to, the points are pointers to vectors
 */
void make_rectangle_points(vector_t corner, list_t *points){
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

/**
 * Generates the list of points for a platform shape given the vector of the bottom left
 * corner of the platform
 *
 * @param corner a vector that contains the coordinates of the bottom left corner of
 * the platform
 * @param points an empty list to add the points to, the points are pointers to vectors
 */
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

/**
 * Initializes a single wall or platform given the info about the body
 * 
 * @param wall_info the object type of the body
*/
list_t *make_rectangle(void *wall_info) {
  vector_t corner = VEC_ZERO;
  body_type_t *info = wall_info;

  if (*info == LEFT_WALL) {
    corner = MIN;
  } 
  if (*info == RIGHT_WALL) {
    corner = (vector_t){MAX.x - WALL_WIDTH.x, MIN.x};
  }
  if (*info == PLATFORM) {
    corner = (vector_t){MIN.x + WALL_WIDTH.x, PLATFORM_HEIGHT};
  }
  list_t *c = list_init(WALL_POINTS, free);
  if (*info == LEFT_WALL || *info == RIGHT_WALL){
    make_rectangle_points(corner, c);
  } else {
    make_platform_points(corner, c);
  }
  
  return c;
}

/**
 * Generates the list of points for a powerup shape given the size of the powerup and
 * the relative location in the vertical direction.
 *
 * @param length corresponds to the length/width of the generated powerup
 * @param power_up_y_loc the relative location of the powerup in the y direction
 * @return list_t containing points of the powerup
*/
list_t *make_power_up_shape(double length, double power_up_y_loc) {
  // randomize location in y direction
  double loc_y = (double) (rand() % ((size_t) POWERUP_LOC));
  loc_y += power_up_y_loc;

  vector_t center = {((MAX.x / 2) - 2 * POWERUP_LOC) + VERTICAL_OFFSET, 
                     loc_y + ((MAX.y / 2) - POWERUP_LOC)};

  list_t *c = list_init(USER_NUM_POINTS, free);
  for (size_t i = 0; i < USER_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / USER_NUM_POINTS;
    vector_t *v = malloc(sizeof(*v));
    assert(v);
    *v = (vector_t){center.x + length * cos(angle),
                    center.y + length * sin(angle)};
    list_add(c, v);
  }
  return c;
}

void create_user(state_t *state) {
  list_t *points = make_user();
  body_t *user = body_init_with_info(points, USER_MASS, USER_COLOR, 
                                     make_type_info(USER), NULL);
  state->user = user;
  body_add_force(user, GRAVITY);
  state->user_health = FULL_HEALTH;
}

/**
 * Initializes both walls and platforms and adds to scene.
 * 
 * @param state the current state of the demo
 * 
*/
void create_walls_and_platforms(state_t *state) {
  scene_t *scene = state -> scene;
  for (size_t i = 0; i < NUM_LEVELS; i++){
    list_t *left_points = make_rectangle(make_type_info(LEFT_WALL));
    list_t *right_points = make_rectangle(make_type_info(RIGHT_WALL));
    body_t *left_wall = body_init_with_info(left_points, WALL_MASS, 
                                            USER_COLOR, make_type_info(LEFT_WALL), 
                                            NULL);
    body_t *right_wall = body_init_with_info(right_points, INFINITY, 
                                            USER_COLOR, make_type_info(RIGHT_WALL), 
                                            NULL);
    scene_add_body(scene, left_wall);
    scene_add_body(scene, right_wall);
    asset_t *wall_asset_l = asset_make_image_with_body(WALL_PATH, left_wall, VERTICAL_OFFSET);
    asset_t *wall_asset_r = asset_make_image_with_body(WALL_PATH, right_wall, VERTICAL_OFFSET);
    list_add(state->body_assets, wall_asset_l);
    list_add(state->body_assets, wall_asset_r);
  }
  list_t *platform_points = make_rectangle(make_type_info(PLATFORM));
  body_t *platform = body_init_with_info(platform_points, INFINITY, 
                                            USER_COLOR, make_type_info(PLATFORM), 
                                            NULL);
  scene_add_body(scene, platform);
  asset_t *wall_asset_platform = asset_make_image_with_body(PLATFORM_PATH, platform, VERTICAL_OFFSET);
  list_add(state->body_assets, wall_asset_platform);

  state->collided_obj = platform; // inital start location
}

/**
 * Creates a jump power up and adds to state
 * @param state the current state of the demo
 * @param powerup_path the path to the powerup file
*/
void create_jump_power_up(state_t *state) {
  list_t *points = make_power_up_shape(POWERUP_LENGTH, JUMP_POWERUP_LOC);
  body_t *powerup = body_init_with_info(points, POWERUP_MASS, USER_COLOR, 
                                       make_type_info(JUMP_POWER), NULL);
  asset_t *powerup_asset = asset_make_image_with_body(JUMP_POWERUP_PATH, powerup, state->vertical_offset);
  state->jump_powerup_index = list_size(state->body_assets);
  list_add(state->body_assets, powerup_asset);
  scene_add_body(state->scene, powerup);
}

/**
 * Creates a health power up and adds to state
 * @param state the current state of the demo
 * @param powerup_path the path to the powerup file
*/
void create_health_power_up(state_t *state) {
  list_t *points = make_power_up_shape(POWERUP_LENGTH, HEALTH_POWERUP_LOC);
  body_t *powerup = body_init_with_info(points, POWERUP_MASS, USER_COLOR, 
                                       make_type_info(HEALTH_POWER), NULL);
  asset_t *powerup_asset = asset_make_image_with_body(HEALTH_POWERUP_PATH, powerup, state->vertical_offset);
  state->health_powerup_index = list_size(state->body_assets);
  list_add(state->body_assets, powerup_asset);
  scene_add_body(state->scene, powerup);
}

/**
 * Called whenever the user health changes so that the health bar asset can be updated
 * 
 * @param state state object representing the current demo state
*/
void update_health_bar(state_t *state) {
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, HEALTH_BAR_BOX);
  
  if (state->user_health == 1) {
    health_bar_asset = asset_make_image(HEALTH_BAR_1_PATH, HEALTH_BAR_BOX);
  } else if (state->user_health == 2) {
    health_bar_asset = asset_make_image(HEALTH_BAR_2_PATH, HEALTH_BAR_BOX);
  }
  state->health_bar = health_bar_asset;
}

// /**
//  * Implements a buffer for the user's jumps off the platform and wall
//  * 
//  * @param state state object representing the current demo state
// */
// void check_jump_off(state_t *state) {
//   if (state->can_jump < JUMP_BUFFER) {
//     state->can_jump++;
//   } else {
//     state->jumping = true;
//   }
// }

/**
 * Check whether two bodies are colliding and applies a sticky collision between them
 * and to be called every tick
 *
 * @param state state object representing the current demo state
 * @param body1 the user
 * @param body2 the body with which the user is colliding
 */
void sticky_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const){

  state_t *state = aux;
  physics_collision_handler(body1, body2, axis, aux, force_const);
  
  state->jumping = false;
  state->can_jump = 0;
  state->collided_obj = body2;
  // state_t *state = aux;
  // vector_t v1 = body_get_velocity(body1);
  // vector_t v2 = body_get_velocity(body2);
  // state -> collided = find_collision(body1, body2).collided;

  // // Check if either velocity is not 0 so that the body's velocities aren't redundantly set to 0
  // bool velocity_zero = (vec_cmp(v1, VEC_ZERO) && vec_cmp(v2, VEC_ZERO)); 

  // if (state -> collided && !velocity_zero){
  //   state->collided_obj = get_type(body2);
  //   body_set_velocity(body1, VEC_ZERO);
  //   body_set_velocity(body2, VEC_ZERO);
  //   state->jumping = false;
  //   state->can_jump = 0;
  //   if (get_type(body2) == PLATFORM) {
  //     body_set_velocity(body1, (vector_t) {v1.x * PLATFORM_FRICTION, 0});
  //   }
  // } else {
  //   state->collided_obj = NONE;
  //   body_add_force(body1, GRAVITY);
  // }
}

void health_powerup_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  state_t *state = aux;
  body_remove(body2);
  list_remove(state->body_assets, state->health_powerup_index);
    if (state->user_health < FULL_HEALTH) {
      state->user_health++;
      update_health_bar(state);
  }

  if (state->jump_powerup_index > state->health_powerup_index) {
    state->jump_powerup_index--;
  }
}

void jump_powerup_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  state_t *state = aux;
  body_remove(body2);
  list_remove(state->body_assets, state->jump_powerup_index);
  state->jump_powerup = true;

  if (state->health_powerup_index > state->jump_powerup_index) {
    state->health_powerup_index--;
  }
}

void jump_powerup_run(state_t *state, double dt) {
  if (state->jump_powerup) {
    if (state->jump_powerup_time < jump_powerup_time) {
      state->jump_powerup_time += dt;
    } else {
      state->jump_powerup = false;
      state->jump_powerup_time = 0;
    }
  }
}

/**
 * Adds collision handler force creators between appropriate bodies.
 *
 * @param state the current state of the demo
 */
void add_force_creators(state_t *state) { 
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    switch (get_type(body)) {
    case LEFT_WALL:
      create_collision(state->scene, state->user, body,
                       (collision_handler_t)sticky_collision, state, WALL_ELASTICITY);
      break;
    case RIGHT_WALL:
      create_collision(state->scene, state->user, body,
                       (collision_handler_t)sticky_collision, state, WALL_ELASTICITY);
      break;
    case PLATFORM:
      create_collision(state->scene, state->user, body,
                       (collision_handler_t)sticky_collision, state, WALL_ELASTICITY);
      break;
    case JUMP_POWER:
      create_collision(state->scene, state->user, body,
                       (collision_handler_t)jump_powerup_collision, state, POWERUP_ELASTICITY);
      break;
    case HEALTH_POWER:
      create_collision(state->scene, state->user, body,
                       (collision_handler_t)health_powerup_collision, state, POWERUP_ELASTICITY);
      break;
    default:
      break;
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
  body_t *user = state->user;
  vector_t cur_v = body_get_velocity(user);
  double new_vx = cur_v.x;
  double new_vy = cur_v.y;

  if (type == KEY_PRESSED) {
      switch (key) {
      case LEFT_ARROW: {
        if (get_type(state->collided_obj) != LEFT_WALL) {
          new_vx = -1 * (RESTING_SPEED + ACCEL * held_time);
        }
        break;
      }
      case RIGHT_ARROW: {
        if (get_type(state->collided_obj) != RIGHT_WALL) {
          new_vx = RESTING_SPEED + ACCEL * held_time;
        }
        break;
      }
      case UP_ARROW: {
        if (!state->jumping || state->jump_powerup) {
          new_vy = USER_JUMP_HEIGHT;
          state->jumping = true;
        }
        break;
      }
    }
  }
  body_set_velocity(user, (vector_t) {new_vx, new_vy});
}

void check_jump(state_t *state) {
  // implement buffer for user's jumps off walls and platform
  if (state->jumping) {
    state->collided_obj = NULL;
    body_add_force(state->user, GRAVITY);
  } else {
    body_reset(state->user);
    // double user_xpos = body_get_centroid(state->user).x;
    // double obj_xpos = body_get_centroid(state->collided_obj).x;
    // if (fabs(user_xpos - obj_xpos) > JUMP_BUFFER) {
    //   state->jumping = true;
    // }
  }
}

/**
 * Check conditions to see if game is over. Game is over if the user has no more health
 * (loss), the user falls off the map (loss), or the user reaches the top of the map (win).
 *
 * @param state a pointer to a state object representing the current demo state
 */
bool game_over(state_t *state) {
  return false;
} 

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  asset_cache_init();
  state_t *state = malloc(sizeof(state_t));
  assert(state);

  // Initialize scene
  state->scene = scene_init();
  state->body_assets = list_init(BODY_ASSETS, (free_func_t)asset_destroy);
  
  // Initialize background
  SDL_Rect background_box = {.x = MIN.x, .y = MIN.y, .w = MAX.x, .h = MAX.y};
  asset_t *background_asset = asset_make_image(BACKGROUND_PATH, background_box);
  list_add(state->body_assets, background_asset);

  // Initialize health bar
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, HEALTH_BAR_BOX);
  state->health_bar = health_bar_asset;
  update_health_bar(state);

  // Initialize user
  create_user(state);
  asset_t *user_asset = asset_make_image_with_body(USER_PATH, state->user, state->vertical_offset);
  list_add(state->body_assets, user_asset);

  // Intialize walls and platforms
  create_walls_and_platforms(state);

  // Initialize powerups
  create_health_power_up(state);
  create_jump_power_up(state);
  state->jump_powerup = false;
  state->jump_powerup_time = 0;

  // Initialize miscellaneous state values
  state->game_over = false;
  state->vertical_offset = 0;
  state->jumping = false;
  state->can_jump = 0;
  
  add_force_creators(state);
  sdl_on_key((key_handler_t)on_key);

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  body_t *user = state->user;
  scene_t *scene = state->scene;
  scene_tick(scene, dt);
  body_tick(user, dt);
  sdl_clear();

  check_jump(state);

  // check if jump powerup is running and update if so
  jump_powerup_run(state, dt);

  vector_t player_pos = body_get_centroid(user);
  state->vertical_offset = player_pos.y - VERTICAL_OFFSET;

  // render assets
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i), state->vertical_offset);
  }
  asset_render(state->health_bar, state->vertical_offset);

  sdl_show(state->vertical_offset);

  // // collisions between walls, platforms, powerups and user
  // for (size_t i = 0; i < scene_bodies(scene); i++){
  //   body_t *body = scene_get_body(scene, i);

  //   // include gravity
  //   if ((!find_collision(state -> user, body).collided && get_type(body) == PLATFORM) || !state->collided){
  //     body_add_force(state -> user, GRAVITY);
  //   }
  // }

  return game_over(state);
}

void emscripten_free(state_t *state) {
  TTF_Quit();
  scene_free(state->scene);
  list_free(state->body_assets);
  body_free(state->user);
  asset_cache_destroy();
  free(state);
}


/**
 * Notes
 * - Why power up isn't working
 *    - What is the purpose of state->body_assets? 
 *    - Powerups can't be removed because there is no way to remove from body_assets
 *      after removing from state->scene
 * Just like record the index of the body asset in the body asset list
*/