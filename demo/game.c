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
const char *GHOST_PATH = "assets/ghost.png";
const char *SPIKE_PATH = "assets/spike.png";

const char *GHOST_HIT_PATH = "assets/ghost_hit.wav";
const char *FLYING_PATH = "assets/flying.wav";
const char *SPIKE_IMPACT_PATH = "assets/spike_impact.wav";
const char *PLATFORM_IMPACT_PATH = "assets/platform_land.wav";
const char *WALL_IMPACT_PATH = "assets/wall_impact.wav";

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
const size_t FULL_HEALTH = 3;
const size_t ZERO_SEED;

// Ghost constants
const double GHOST_RADIUS = 30;
const double GHOST_NUM = 3; // Total number of ghosts to be spawned 
const double GHOST_MASS = 5;
const rgb_color_t GHOST_COLOUR = (rgb_color_t){0, 0, 0};
const double GHOST_OFFSET = 50;
const double GHOST_SPEED = 150;
const double SPAWN_TIMER = 3;
const size_t INITIAL_GHOST = 1;
const double Y_OFFSET_GHOST = -100;
const double VELOCITY_BUFFER = 0.1;
const double GHOST_ELASTICITY = 1;
const double TRANSLATE_SCALE = 800;
const double GHOST_RAND_MAX = 20;
const size_t Y_RAND = 413;
const double RAND_SPEED = 80;
const vector_t RAND_VELOCITY = {60, 60};
const size_t IMMUNITY = 3;

// Obstacle constants
const double SPIKE_RADIUS = 120;
const vector_t SPIKE_MIN = {150, 500};
const vector_t SPIKE_MAX = {600, 0};
const double SPIKE_MASS = 5;
const size_t SPIKE_NUM = 6;

// Wall constants
const vector_t WALL_WIDTH = {100, 0};
const vector_t WALL_LENGTH = {0, 2000};
const size_t WALL_POINTS = 4;
const double WALL_MASS = INFINITY;
const double WALL_ELASTICITY = 0;
const size_t TEMP_LENGTH = 3;
const double NORMAL_SCALING = 1;
const double PLATFORM_SCALING = 5;
const double PLATFORM_HEIGHT = 100;
const vector_t PLATFORM_LENGTH = {0, 15};
const vector_t PLATFORM_WIDTH = {110, 0};
const double PLATFORM_FRICTION = .85;
const size_t PLATFORM_LEVEL = 0;

// health bar location
const vector_t HEALTH_BAR_MIN = {15, 15};
const vector_t HEALTH_BAR_MAX = {90, 30};
SDL_Rect HEALTH_BAR_BOX = {.x = HEALTH_BAR_MIN.x, .y = HEALTH_BAR_MIN.y, 
                           .w = HEALTH_BAR_MAX.x, .h = HEALTH_BAR_MAX.y};

// powerup constants
const size_t POWERUP_LOC = 50; // radius from tower center where powerups generated
const size_t JUMP_POWERUP_LOC = (size_t) 2 * (MAX.y / 3);
const size_t HEALTH_POWERUP_LOC = (size_t) (MAX.y / 3);
const double POWERUP_LENGTH = 18;
const double POWERUP_MASS = .0001;
const double POWERUP_ELASTICITY = 1;
const size_t JUMP_POWERUP_JUMPS = 2;

// Sound constants
const size_t SOUND_SIZE = 5;

// Game constants
const size_t NUM_LEVELS = 3;
const vector_t GRAVITY = {0, -1000};
const size_t BODY_ASSETS = 3; // total assets, 2 walls and 1 platform
const double BACKGROUND_CORNER = 150;
const double VERTICAL_OFFSET = 100;

typedef enum { USER, LEFT_WALL, RIGHT_WALL, PLATFORM, JUMP_POWER, HEALTH_POWER, GHOST, SPIKE, NONE } body_type_t;

typedef enum { GHOST_IMPACT, FLYING, SPIKE_IMPACT, PLATFORM_IMPACT, WALL_IMPACT } sound_type_t;

typedef struct sound {
  Mix_Chunk *player;
  sound_type_t *info;
} sound_t;

struct state {
  scene_t *scene;
  list_t *body_assets;
  asset_t *user_sprite;
  body_t *user;
  
  size_t user_health;
  asset_t *health_bar;
  double user_immunity;

  size_t ghost_counter;
  double ghost_timer;
  double velocity_timer;

  double vertical_offset;
  bool game_over;
  
  bool jumping; // determines whether up button can be pressed
  body_t *collided_obj; // the object that the user is collided with
  
  size_t jump_powerup_jumps;

  size_t jump_powerup_index;
  size_t health_powerup_index;

  list_t *sounds;
};

void sound_free(sound_t *sound){
  Mix_FreeChunk(sound->player);
  free(sound);
}

void sound_init(state_t *state){
  list_t *sounds = list_init(SOUND_SIZE, (free_func_t) sound_free);
  const char* paths[] = {GHOST_HIT_PATH, FLYING_PATH, SPIKE_IMPACT_PATH, 
                        PLATFORM_IMPACT_PATH, WALL_IMPACT_PATH};
  for (size_t i = 0; i < SOUND_SIZE; i++){
    sound_t *sound = malloc(sizeof(sound_t));
    sound_type_t *info = malloc(sizeof(sound_type_t));
    *info = i;
    sound->info = info;
    sound->player = sdl_load_sound(paths[i]);
    list_add(sounds, sound);
  }
  state->sounds = sounds;
}

Mix_Chunk *get_sound(state_t *state, sound_type_t sound_type){
  list_t* sounds = state->sounds;
  for (size_t i = 0; i < SOUND_SIZE; i++){
    sound_t *sound = list_get(sounds, i);
    if (*(sound->info) == sound_type){
      return sound->player;
    }
  }
}


/**
 * Get body_type_t type of body
 * 
 * @param body body to find the fype of
 * @return body_type_t associated with body
*/
body_type_t get_type(body_t *body) {
  if (body == NULL) {
    return NONE;
  }
  return *(body_type_t *)body_get_info(body);
}

/**
 * Covert body type into pointer
 * 
 * @param type body_type_t to be converted
 * @return body_type_t* pointer to type
*/
body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  *info = type;
  return info;
}



/**
 * Creates user shape.
 * 
 * @return list_t containing the points of the shape
*/
list_t *make_user(vector_t center, void *info, size_t seed) {
  double radius = RADIUS;
  vector_t center_body = center;
  if (*(body_type_t *)info == SPIKE){
    radius = SPIKE_RADIUS;
    vector_t spike_max = {SPIKE_MAX.x, SPIKE_MAX.y + 
                          WALL_LENGTH.y * NUM_LEVELS};
    center_body = rand_vec(SPIKE_MIN, spike_max, seed);

  }
  list_t *c = list_init(USER_NUM_POINTS, free);
  for (size_t i = 0; i < USER_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / USER_NUM_POINTS;
    vector_t *v = malloc(sizeof(*v));
    assert(v);
    *v = (vector_t){center_body.x + radius * cos(angle),
                    center_body.y + radius * sin(angle)};
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
void make_rectangle_points(vector_t corner, list_t *points, bool platform){
  vector_t temp[] = {PLATFORM_LENGTH, PLATFORM_WIDTH, vec_negate(PLATFORM_LENGTH)};
  if (!platform){
    temp[0] = WALL_LENGTH;
    temp[1] = WALL_WIDTH;
    temp[2] = vec_negate(WALL_LENGTH);
  }
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
  make_rectangle_points(corner, points, true);
}

/**
 * Initializes a single wall or platform given the info about the body
 * 
 * @param wall_info the object type of the body
*/
list_t *make_rectangle(void *wall_info, size_t level) {
  vector_t corner = VEC_ZERO;
  body_type_t *info = wall_info;

  if (*info == LEFT_WALL) {
    corner = (vector_t){MIN.x, MIN.y + WALL_LENGTH.y * level};
  } 
  if (*info == RIGHT_WALL) {
    corner = (vector_t){MAX.x - WALL_WIDTH.x, MIN.y + WALL_LENGTH.y * level};
  }
  if (*info == PLATFORM) {
    corner = (vector_t){MIN.x + WALL_WIDTH.x, PLATFORM_HEIGHT};
  }
  list_t *c = list_init(WALL_POINTS, free);
  if (*info == LEFT_WALL || *info == RIGHT_WALL){
    make_rectangle_points(corner, c, false);
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
  vector_t center = {MIN.x + RADIUS + WALL_WIDTH.x, 
                    MIN.y + RADIUS + PLATFORM_HEIGHT + PLATFORM_LENGTH.y};
  list_t *points = make_user(center,  make_type_info(USER), ZERO_SEED);
  body_t *user = body_init_with_info(points, USER_MASS, USER_COLOR, 
                                    make_type_info(USER), NULL);
  state->user = user;
  body_add_force(user, GRAVITY);
  state->jumping = false;
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
    list_t *left_points = make_rectangle(make_type_info(LEFT_WALL), i);
    list_t *right_points = make_rectangle(make_type_info(RIGHT_WALL), i);
    body_t *left_wall = body_init_with_info(left_points, WALL_MASS, 
                                            USER_COLOR, make_type_info(LEFT_WALL), 
                                            NULL);
    body_t *right_wall = body_init_with_info(right_points, WALL_MASS, 
                                            USER_COLOR, make_type_info(RIGHT_WALL), 
                                            NULL);
    scene_add_body(scene, left_wall);
    scene_add_body(scene, right_wall);
    asset_t *wall_asset_l = asset_make_image_with_body(WALL_PATH, left_wall, VERTICAL_OFFSET);
    asset_t *wall_asset_r = asset_make_image_with_body(WALL_PATH, right_wall, VERTICAL_OFFSET);
    list_add(state->body_assets, wall_asset_l);
    list_add(state->body_assets, wall_asset_r);
  }
  list_t *platform_points = make_rectangle(make_type_info(PLATFORM), PLATFORM_LEVEL);
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
  state->jump_powerup_jumps = 0;
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

/**
 * Check whether two bodies are colliding and applies a sticky collision between them
 * and to be called every tick
 *
 * @param body1 the user
 * @param body2 the body with which the user is colliding
 * @param axis the axis of collision
 * @param aux information about the state of the collision
 * @param force_const the force constant to be applied to the collision
 */
void sticky_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const){

  state_t *state = aux;
  physics_collision_handler(body1, body2, axis, aux, force_const);
  
  state->jumping = false;
  state->collided_obj = body2;
}

/**
 * Collision handler for health powerups
 *
 * @param body1 the user
 * @param body2 the body with which the user is colliding
 * @param axis the axis of collision
 * @param aux information about the state of the collision
 * @param force_const the force constant to be applied to the collision
 */
void health_powerup_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  state_t *state = aux;
  body_remove(body2);
  list_remove(state->body_assets, state->health_powerup_index);

  // add to health only if health is not full
  if (state->user_health < FULL_HEALTH) {
      state->user_health++;
      update_health_bar(state);
  }

  if (state->jump_powerup_index > state->health_powerup_index) {
    state->jump_powerup_index--;
  }
}

/**
 * Collision handler for jump powerups
 *
 * @param body1 the user
 * @param body2 the body with which the user is colliding
 * @param axis the axis of collision
 * @param aux information about the state of the collision
 * @param force_const the force constant to be applied to the collision
 */
void jump_powerup_collision(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  state_t *state = aux;
  body_remove(body2);
  list_remove(state->body_assets, state->jump_powerup_index);

  // set number of extra jumps the user can take
  state->jump_powerup_jumps = JUMP_POWERUP_JUMPS;

  if (state->health_powerup_index > state->jump_powerup_index) {
    state->health_powerup_index--;
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
void ghost_collision(body_t *user, body_t *body, vector_t axis, void *aux,
                double force_const){
  state_t *state = aux;
  if (state -> user_immunity > IMMUNITY){
    if (state -> user_health > 1){
      state -> user_health --;
      update_health_bar(state);
      sdl_play_sound(get_sound(state, GHOST_IMPACT));
    } else {
      //body_remove(user);
    }
    state -> user_immunity = 0;
  }
}



/**
 * Spawns a ghost on the screen at fixed y value and at a random x value
 * that is within the bounds of the window
 * 
 */
void spawn_ghost(state_t *state) {
  
  vector_t max = {MAX.x, 0};
  double x = rand_vec(VEC_ZERO, max, ZERO_SEED).x;
  vector_t ghost_center = {x, Y_OFFSET_GHOST};
  list_t *c = make_user(ghost_center, make_type_info(GHOST), ZERO_SEED);
  body_t *ghost = body_init_with_info(c, GHOST_MASS, GHOST_COLOUR, 
                                      make_type_info(GHOST), NULL);
  scene_add_body(state -> scene, ghost);
  asset_t *ghost_asset = asset_make_image_with_body(GHOST_PATH, ghost, VERTICAL_OFFSET);
  list_add(state->body_assets, ghost_asset);
  create_collision(state->scene, state->user, ghost,
                      (collision_handler_t)ghost_collision, state, 0);
  state -> ghost_counter++;
  state -> ghost_timer = 0;
  
}

/**
 * Spawns a ghost on the screen at fixed y value and at a random x value
 * that is within the bounds of the window
 */
void ghost_move(state_t *state){
  scene_t *scene = state->scene;
  size_t num_bodies = scene_bodies(scene);
  for (size_t i = 0; i < num_bodies; i++){
    body_t *body = scene_get_body(scene, i);
    if (get_type(body) == GHOST && (state->velocity_timer > VELOCITY_BUFFER)){
        vector_t user_center = body_get_centroid(state->user);
        vector_t ghost_center = body_get_centroid(body);
        vector_t direction = vec_unit(vec_add(user_center, vec_negate(ghost_center)));
        vector_t velocity = vec_multiply(GHOST_SPEED, direction);
        vector_t rand_add = rand_vec(VEC_ZERO, RAND_VELOCITY, i);
        vector_t rand_velocity = vec_add(velocity, rand_add);
        body_set_velocity(body, rand_velocity);
        if (i == num_bodies - 1){
          state->velocity_timer = 0;
        }
      }
    }
}


/**
 * Spawns spikes on the screen at random y value and at a random x value
 * that is within the bounds wall height and the space in between
 */
void spawn_spike(state_t *state) {
  for (size_t i = 0; i < SPIKE_NUM; i++){
    list_t *c = make_user(VEC_ZERO, make_type_info(SPIKE), i);
    body_t *spike = body_init_with_info(c, SPIKE_MASS, GHOST_COLOUR, 
                                        make_type_info(SPIKE), NULL);
    scene_add_body(state -> scene, spike);
    asset_t *spike_asset = asset_make_image_with_body(SPIKE_PATH, spike, VERTICAL_OFFSET);
    list_add(state->body_assets, spike_asset);
  }
}

/**
 * Helper function to check when gravity should be applied to the user
 * 
 * @param state the current state of the demo
*/
void check_jump(state_t *state) {
  // adds gravity if user is in the air
  if (state->jumping) {
    state->collided_obj = NULL;
    body_add_force(state->user, GRAVITY);
  } 

  // removes gravity if user is not in the air
  else {
    body_reset(state->user);

    bool is_collided = false;

    // check if the user is still collided
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *body = scene_get_body(state->scene, i);
      if (find_collision(state->user, body).collided) {
        is_collided = true;
        break;
      }
    }
    
    // determines whether the user has fallen and can no longer jump
    if (is_collided == false) {
      double user_xpos = body_get_centroid(state->user).x;
      double obj_xpos = body_get_centroid(state->collided_obj).x;
      if (fabs(user_xpos - obj_xpos) > JUMP_BUFFER) {
        state->jumping = true;
      }
    }
  }
}

/**
 * Helper function to add gravity and friction in edge cases
 * 
 * @param state the current state of the demo
*/
void check_gravity_and_friction(state_t *state) {
  check_jump(state);

  // add horizontal friction if the user is on the platform
  if (get_type(state->collided_obj) == PLATFORM) {
    vector_t v1 = body_get_velocity(state->user);
    body_set_velocity(state->user, (vector_t) {v1.x * PLATFORM_FRICTION, 0});
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
    case SPIKE:
      create_collision(state->scene, state->user, body, 
                      (collision_handler_t)ghost_collision, state, GHOST_ELASTICITY);
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
        if (!state->jumping || state->jump_powerup_jumps > 0) {
          new_vy = USER_JUMP_HEIGHT;

          // determine if jump was taken using powerup and update if so
          if (state->jumping && state->jump_powerup_jumps > 0) {
            state->jump_powerup_jumps--;
          }
          state->jumping = true;
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

  // Initialize sound
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  Mix_Volume(-1, MIX_MAX_VOLUME);

  
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

  // Initialize obstacles
  spawn_spike(state);
  

  // Initialize miscellaneous state values
  state->game_over = false;
  state->vertical_offset = 0;
  state->velocity_timer = 0;
  state->ghost_counter = 0;
  state->user_immunity = 0;
  state->ghost_timer = 0;
  
  add_force_creators(state);
  sdl_on_key((key_handler_t)on_key);

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state -> ghost_timer += dt;
  state -> velocity_timer += dt;
  state -> user_immunity += dt;
  body_t *user = state->user;
  scene_t *scene = state->scene;
  scene_tick(scene, dt);
  body_tick(user, dt);
  sdl_clear();

  check_gravity_and_friction(state);

  vector_t player_pos = body_get_centroid(user);
  state->vertical_offset = player_pos.y - VERTICAL_OFFSET;

  // spawn and move ghosts
  if (state -> ghost_timer > SPAWN_TIMER && state -> ghost_counter <= GHOST_NUM){
    spawn_ghost(state);
  }
  ghost_move(state);

  // render assets
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i), state->vertical_offset);
  }
  asset_render(state->health_bar, state->vertical_offset);

  sdl_show(state->vertical_offset);

  return game_over(state);
}

void emscripten_free(state_t *state) {
  TTF_Quit();
  list_free(state->sounds);
  scene_free(state->scene);
  list_free(state->body_assets);
  body_free(state->user);
  asset_cache_destroy();
  free(state);
}