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
const char *WALL_PATH = "assets/wall.jpeg";
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
const size_t WALL_JUMP_BUFFER = 30; // how many pixels away from wall can user jump
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
const vector_t PLATFORM_LENGTH = {0, 10};
const vector_t PLATFORM_WIDTH = {100, 0};
const double PLATFORM_ROTATION = M_PI/2;
const double PLATFORM_FRICTION = .95;

// health bar location
const vector_t HEALTH_BAR_MIN = {15, 15};
const vector_t HEALTH_BAR_MAX = {90, 30};
SDL_Rect HEALTH_BAR_BOX = {.x = HEALTH_BAR_MIN.x, .y = HEALTH_BAR_MIN.y, 
                           .w = HEALTH_BAR_MAX.x, .h = HEALTH_BAR_MAX.y};

// powerup constants
const size_t POWERUP_LOC = 50; // radius from tower center where powerups generated
const size_t JUMP_POWERUP_LOC = (size_t) 2 * (MAX.y / 3);
const size_t HEALTH_POWERUP_LOC = (size_t) (MAX.y / 3);
const double POWERUP_TIME = 7; // how long jump powerup lasts
const double POWERUP_LENGTH = 18;
const double POWERUP_MASS = .0001;
const double POWERUP_ELASTICITY = 1;

// Game constants
const size_t NUM_LEVELS = 1;
const vector_t GRAVITY = {0, -1200};
const size_t BODY_ASSETS = 3; // total assets, 2 walls and 1 platform
const double BACKGROUND_CORNER = 150;
const double VERTICAL_OFFSET = 100;

typedef enum { USER, LEFT_WALL, RIGHT_WALL, PLATFORM, JUMP_POWER, HEALTH_POWER } body_type_t;

struct state {
  scene_t *scene;
  list_t *body_assets;
  asset_t *user_sprite;
  body_t *user_body;
  size_t user_health;
  asset_t *health_bar;

  size_t ghost_counter;
  double ghost_timer;
  double vertical_offset;
  bool game_over;
  
  bool collided;
  bool jumping;
  size_t can_jump;
  
  bool jump_powerup;
  double powerup_time;
};

body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  *info = type;
  return info;
}

/**
 * Sets the velocity of the user so that the user can jump from sticky walls
 * 
 * @param state state object representing the current demo state
 * @param velocity velocity to set the user to 
 */
void set_velocity(state_t *state, vector_t velocity){
  body_t *user = state -> user_body;
  body_set_velocity(user, velocity);
  vector_t center = body_get_centroid(state -> user_body);
  vector_t move = {velocity.x/VELOCITY_SCALE, velocity.y/VELOCITY_SCALE};
  body_set_centroid(user, vec_add(center, move));
}

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
list_t *make_wall(void *wall_info) {
  vector_t corner = VEC_ZERO;
  body_type_t *info = wall_info;
  // size_t cmp_left = strcmp(wall_info, LEFT_WALL_INFO);
  // size_t cmp_right = strcmp(wall_info, RIGHT_WALL_INFO);
  // size_t cmp_plat = strcmp(wall_info, PLATFORM_INFO);

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
    make_wall_points(corner, c);
  } else {
    make_platform_points(corner, c);
  }
  
  return c;
}

/**
 * Initializes both walls and platforms and adds to scene.
 * 
 * @param state the current state of the demo
 * 
*/
void wall_init(state_t *state) {
  scene_t *scene = state -> scene;
  for (size_t i = 0; i < NUM_LEVELS; i++){
    list_t *left_points = make_wall(make_type_info(LEFT_WALL));
    list_t *right_points = make_wall(make_type_info(RIGHT_WALL));
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
  list_t *platform_points = make_wall(make_type_info(PLATFORM));
  body_t *platform = body_init_with_info(platform_points, INFINITY, 
                                            USER_COLOR, make_type_info(PLATFORM), 
                                            NULL);
  scene_add_body(scene, platform);
  //create_collision(scene, platform, state -> user_body, physics_collision_handler, (char*)"v_0", WALL_ELASTICITY);
  asset_t *wall_asset_platform = asset_make_image_with_body(PLATFORM_PATH, platform, VERTICAL_OFFSET);
  list_add(state->body_assets, wall_asset_platform);
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
  //create_collision(state->scene, powerup, state->user_body, physics_collision_handler, 
                  //(char*)"v_0", POWERUP_ELASTICITY);
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
  //create_collision(state->scene, powerup, state->user_body, (void *) physics_collision_handler, 
                  //(char*)"v_0", POWERUP_ELASTICITY);
  list_add(state->body_assets, powerup_asset);
  scene_add_body(state->scene, powerup);
}

/**
 * Called whenever the user health changes so that the health bar asset can be updated
 * 
 * @param state state object representing the current demo state
*/
void health_bar_process(state_t *state) {
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, HEALTH_BAR_BOX);
  
  if (state->user_health == 1) {
    health_bar_asset = asset_make_image(HEALTH_BAR_1_PATH, HEALTH_BAR_BOX);
  } else if (state->user_health == 2) {
    health_bar_asset = asset_make_image(HEALTH_BAR_2_PATH, HEALTH_BAR_BOX);
  }
  state->health_bar = health_bar_asset;
}

/**
 * Implements a buffer for the user's jumps off the platform and wall
 * 
 * @param state state object representing the current demo state
*/
void check_jump_off(state_t *state) {
  if (state->can_jump < WALL_JUMP_BUFFER) {
    state->can_jump++;
  } else {
    state->jumping = true;
  }
}

/**
 * Check whether two bodies are colliding and applies a sticky collision between them
 * and to be called every tick
 *
 * @param state state object representing the current demo state
 * @param body1 the user
 * @param body2 the body with which the user is colliding
 */
void sticky_collision(state_t *state, body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const){
  vector_t v1 = body_get_velocity(body1);
  vector_t v2 = body_get_velocity(body2);
  state -> collided = find_collision(body1, body2).collided;

  // Check if either velocity is not 0 so that the body's velocities aren't redundantly set to 0
  bool velocity_zero = (vec_cmp(v1, VEC_ZERO) && vec_cmp(v2, VEC_ZERO)); 

  if (state -> collided && !velocity_zero){
    body_set_velocity(body1, VEC_ZERO);
    body_set_velocity(body2, VEC_ZERO);
    state->jumping = false;
    state->can_jump = 0;
    if (get_type(body2) == PLATFORM) {
      body_set_velocity(body1, (vector_t) {v1.x * PLATFORM_FRICTION, 0});
  }
  }
}

/**
 * The collision handler for collisions between the ball and the brick.
 *
 * @param body1 the body for the ball
 * @param body2 the body for the brick
 * @param axis the axis of collision
 * @param aux the aux passed in from `create_breakout_collision`
 * @param elasticity the elasticity of the collision between the ball and the
 * brick
 */
void breakout_collision_handler(state_t *state, body_t *body1, body_t *body2, vector_t axis,
                                void *aux, double force_const) {
  body_remove(body2); // Make brick disappear
  if (state->user_health < 3) {
    state->user_health++;
    health_bar_process;
  }
}

/**
 * The breakout collision creator for `breakout_collision_handler`.
 *
 * @param scene the scene of the game
 * @param body1 the body for the ball
 * @param body2 the body for the brick
 * @param elasticity the elasticity of the collision between the ball and the
 * brick
 */
void create_breakout_collision(state_t *state, scene_t *scene, body_t *body1, body_t *body2,
                               double elasticity) {
  create_collision(state, scene, body1, body2,
                   (collision_handler_t)breakout_collision_handler, NULL,
                   elasticity);
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
      create_collision(state_t *state, state->scene, state->user_body, body,
                       (collision_handler_t)sticky_collision, state, 0);
      break;
    case RIGHT_WALL:
      create_collision(state_t *state, state->scene, state->user_body, body,
                       (collision_handler_t)sticky_collision, state, 0);
      break;
    case PLATFORM:
      create_collision(state_t *state, state->scene, state->user_body, body,
                       (collision_handler_t)sticky_collision, state, 0);
      break;
    case JUMP_POWER:
      create_breakout_collision(state, state->scene, state->user_body, body, POWERUP_ELASTICITY);
      break;
    case HEALTH_POWER:
      create_breakout_collision(state, state->scene, state->user_body, body, POWERUP_ELASTICITY);
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
  body_t *user = state->user_body;
  vector_t cur_v = body_get_velocity(user);
  double new_vx = cur_v.x;
  double new_vy = cur_v.y;

  if (type == KEY_PRESSED) {
      switch (key) {
      case LEFT_ARROW: {
        new_vx = -1 * (RESTING_SPEED + ACCEL * held_time);
        break;
      }
      case RIGHT_ARROW: {
        new_vx = RESTING_SPEED + ACCEL * held_time;
        break;
      }
      case UP_ARROW: {
        if (!state->jumping || state->jump_powerup) {
          new_vy = USER_JUMP_HEIGHT;
        }
        break;
      }
    }
  }
  body_set_velocity(user, (vector_t) {new_vx, new_vy});
}

/**
 * Check conditions to see if game is over. Game is over if the user has no more health
 * (loss), the user falls off the map (loss), or the user reaches the top of the map (win).
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

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  asset_cache_init();
  state_t *state = malloc(sizeof(state_t));
  assert(state);

  // intialize scene and user
  state->scene = scene_init();
  state->body_assets = list_init(BODY_ASSETS, (free_func_t)asset_destroy);
  list_t *points = make_user();
  state->user_body =
      body_init_with_info(points, USER_MASS, USER_COLOR, make_type_info(USER), NULL);
  body_t* body = state->user_body;
  body_add_force(state -> user_body, GRAVITY);
  state->user_health = FULL_HEALTH;

  // initialize scrolling velocity
  vector_t initial_velocity = {20, 20};
  set_velocity(state, initial_velocity);

  // Create and save the asset for the background image
  SDL_Rect background_box = {.x = MIN.x, .y = MIN.y, .w = MAX.x, .h = MAX.y};
  asset_t *background_asset = asset_make_image(BACKGROUND_PATH, background_box);
  list_add(state->body_assets, background_asset);

  // Create and save the asset for the user image
  asset_t *user_asset = asset_make_image_with_body(USER_PATH, body, state->vertical_offset);
  list_add(state->body_assets, user_asset);

  // create health bar
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, HEALTH_BAR_BOX);
  state->health_bar = health_bar_asset;

  wall_init(state);

  // initialize miscellaneous state values
  state->game_over = false;
  state->collided = false;
  state->vertical_offset = 0;
  state->can_jump = 0;

  // initalize key handler
  sdl_on_key((key_handler_t)on_key);
  
  state->jumping = false;
  state->jump_powerup = false;
  state->powerup_time = 0;

  create_health_power_up(state);
  create_jump_power_up(state);

  add_force_creators(state);

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  body_t *user = state->user_body;
  scene_t *scene = state->scene;
  scene_tick(scene, dt);
  body_tick(user, dt);
  sdl_clear();

  // implement buffer for user's jumps off walls and platform
  if (!state->collided) {
    check_jump_off(state);
  } 

  vector_t player_pos = body_get_centroid(user);
  state->vertical_offset = player_pos.y - VERTICAL_OFFSET;

  // render assets
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i), state->vertical_offset);
  }

  // render health bar
  asset_render(state->health_bar, state->vertical_offset);

  // show all rendered objects
  sdl_show(state->vertical_offset);

  // collisions between walls, platforms, powerups and user
  for (size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);

    //sticky_collision(state, user, body); // determine if user is collided with body

    // include gravity
    if (!find_collision(state -> user_body, body).collided && get_type(body) == PLATFORM){
      body_add_force(state -> user_body, GRAVITY);
    }
  }

  // jump powerup determination
  if (state->jump_powerup) {
    if (state->powerup_time < POWERUP_TIME) {
      state->powerup_time += dt;
    } else {
      state->jump_powerup = false;
      state->powerup_time = 0;
    }
  }

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