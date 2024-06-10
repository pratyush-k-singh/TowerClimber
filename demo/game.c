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
const vector_t MAX = {1000, 1000};
const double HALFWAY_VERTICAL_DISTANCE = 2600;

// Filepaths
const char *BACKGROUND_PATH = "assets/background.png";
const char *VICTORY_BACKGROUND_PATH = "assets/victory_background.png";
const char *PAUSE_BUTTON_PATH = "assets/pause_button.png";
const char *RESET_BUTTON_PATH = "assets/reset_button.png";
const char *START_BUTTON_PATH = "assets/start_button.png";
const char *TITLE_PATH = "assets/title.png";
const char *VICTORY_TEXT_PATH = "assets/victory_text.png";
const char *USER_PATH = "assets/body.png";
const char *WALL_PATH = "assets/wall.png";
const char *PLATFORM_PATH = "assets/platform.png";
const char *JUMP_POWERUP_PATH = "assets/jump_powerup.png";
const char *HEALTH_POWERUP_PATH = "assets/health_powerup.png";
const char *FULL_HEALTH_BAR_PATH = "assets/health_bar_3.png";
const char *HEALTH_BAR_2_PATH = "assets/health_bar_2.png";
const char *HEALTH_BAR_1_PATH = "assets/health_bar_1.png";
const char *HEALTH_BAR_0_PATH = "assets/health_bar_0.png";
const char *GHOST_PATH = "assets/ghost.png";  
const char *GAS_PATH = "assets/obstacle.png";
const char *PORTAL_PATH = "assets/portal.png";
const char *ISLAND_PATH = "assets/island.png";
const char *GHOST_HIT_PATH = "assets/ghost_hit.wav";
const char *WIND_PATH = "assets/wind.wav";
const char *GAS_IMPACT_PATH = "assets/gas_impact.wav";
const char *PLATFORM_IMPACT_PATH = "assets/platform_land.wav";
const char *WALL_IMPACT_PATH = "assets/wall_impact.wav";
const char *MUSIC_PATH = "assets/Pixel-Drama.wav";
const char *SPIKE_PATH = "assets/spike.png";

// User Constants
const double USER_MASS = 5;
const double USER_ROTATION = 0;
const size_t USER_NUM_POINTS = 20;
const double USER_JUMP_HEIGHT = 350;
const rgb_color_t USER_COLOR = (rgb_color_t){0, 0, 0};
const double RADIUS = 25;
const double RESTING_SPEED = 200;
const double VELOCITY_SCALE = 100;
const double ACCEL = 100;
const size_t JUMP_BUFFER = 30; // how many pixels away from wall can user jump
const size_t WALL_BUFFER = 40; // how many pixels away from wall can user move horizontally
const size_t FULL_HEALTH = 3;
const size_t ZERO_SEED;

// Ghost Constants
const double GHOST_RADIUS = 30;
const double GHOST_NUM = 3; // Total number of ghosts to be spawned 
const double GHOST_MASS = 5;
const rgb_color_t GHOST_COLOUR = (rgb_color_t){0, 0, 0};
const double GHOST_OFFSET = 50;
const double GHOST_SPEED = 130;
const double SPAWN_TIMER = 3;
const size_t INITIAL_GHOST = 1;
const double Y_OFFSET_GHOST = -100;
const double VELOCITY_BUFFER = 0.66;
const double GHOST_ELASTICITY = 1;
const double TRANSLATE_SCALE = 800;
const double GHOST_RAND_MAX = 20;
const size_t Y_RAND = 413;
const double RAND_SPEED = 80;
const vector_t RAND_VELOCITY = {80, 80};
const size_t IMMUNITY = 3;
const double RESTART_BUFFER = 5;

// Gas Obstacle Constants
const double GAS_RADIUS = 250;
const vector_t GAS_MIN = {150, 500};
const vector_t GAS_MAX = {600, 0};
const double GAS_MASS = 5;
const size_t GAS_NUM = 6;
const double GAS_OFFSET = 220;

// Portal Constants
const double PORTAL_RADIUS = 350;
const double PORTAL_MASS = 10;
const double PORTAL_ROTATION = 0.05;
const double PORTAL_OFFSET = 300;
const double PORTAL_HEIGHT = 300;
const double PORTAL_VERTICAL_DISTANCE = 5050;

// Quicksand Island Constants
const vector_t ISLAND_LENGTH = {0, 800};
const double ISLAND_LEVEL = 0;
const double ISLAND_MASS = INFINITY;
const double ISLAND_ELASTICITY = 0.36;

// Wall Constants
const vector_t WALL_WIDTH = {100, 0};
const vector_t WALL_LENGTH = {0, 2000};
const size_t WALL_POINTS = 4;
const double WALL_MASS = INFINITY;
const double ELASTICITY = 0;
const rgb_color_t WALL_COLOR = (rgb_color_t) {255, 255, 255};
const size_t TEMP_LENGTH = 3;
const double NORMAL_SCALING = 1;
const double PLATFORM_SCALING = 5;
const double PLATFORM_HEIGHT = 250;
const vector_t PLATFORM_LENGTH = {0, 15};
const vector_t PLATFORM_WIDTH = {110, 0};
const vector_t WALL_FRICTION_FORCE = {0, -500};
const double PLATFORM_FRICTION = .88;
const size_t PLATFORM_LEVEL = 0;
const size_t NUM_PLATFORMS = 5;
const double GAP_DISTANCE = 800;
const size_t MIDDLE = 1;
const size_t WALL_TYPES = 2;

// Health Bar Location
const vector_t HEALTH_BAR_MIN = {15, 15};
const vector_t HEALTH_BAR_MAX = {105, 30};
SDL_Rect HEALTH_BAR_BOX = {.x = HEALTH_BAR_MIN.x, .y = HEALTH_BAR_MIN.y, 
                           .w = HEALTH_BAR_MAX.x, .h = HEALTH_BAR_MAX.y};

// Power-up Constants
const size_t POWERUP_LOC = 50; // radius from tower center where powerups generated
const size_t JUMP_POWERUP_LOC = (size_t) 3 * MAX.y;
const size_t HEALTH_POWERUP_LOC = (size_t) 4 * MAX.y;
const double POWERUP_LENGTH = 18;
const double POWERUP_MASS = .0001;
const double POWERUP_ELASTICITY = 1;
const size_t JUMP_POWERUP_JUMPS = 2;

// Sound Constants
const size_t SOUND_SIZE = 5;
const double HIT_BUFFER = 0.3;
const double COLLIDING_BUFFER = 0.36;
const double FALL_BUFFER = 0.2;
const double FALL_THRESHOLD = 50;
const size_t WIND_CHANNEL = 1;
const size_t IMPACT_CHANNEL = 2;
const size_t FREQUENCY = 44100;
const size_t STEREO = 2;
const size_t AUDIO_BUFFER = 2048;
const size_t DEFAULT_CHANNEL = -1;
const size_t LOOPS = 20;
const size_t MUSIC_VOLUME = 50;

// SPIKE CONSTANTS
const size_t SPIKE1_ENUM = 10;
const size_t SPIKE_RADIUS = 150;
const size_t SPIKE_MASS = 10;
const double SPIKE_OFFSET = 800;
const size_t NUM_SPIKES = 3;
const size_t SPIKE1_INDEX = 0;
const size_t SPIKE2_INDEX = 1;
const size_t SPIKE3_INDEX = 2;
const double SPIKE_ELASTICITY = 0;

// Button and Title Constants
const vector_t TITLE_OFFSETS = {0, 75};
const vector_t VICTORY_OFFSETS = {0, 150};
const vector_t BUTTON_OFFSETS = {0, 300};
const vector_t PAUSE_BUTTON_OFFSETS = {45, 40};

// Messages
const char* WELCOME_MESSAGE = "Welcome to Tower Climber! In this game you are going to have to help the ninja jump to the top of the tower, where the "
                              "mysterious path to the REALM OF EVIL awaits. The Evil King has left ghosts, poisonous clouds, and explosive spike bombs in the way above, and a strange quicksand island below "
                              "in an attempt to stop your ascent, but I doubt they'll stop you for long. Still, that doesn't mean it will be easy, so here is a refresher on how "
                              "to climb:\n\n"

                              "- Horizontal Navigation: Left/Right Arrow Keys\n"
                              "- Jumping: Up Arrow Key\n\n"
                              
                              "Along the way the Goddess was able to scatter a few power-ups to help you. If you're ever injured, just jump into one of the "
                              "floating red hearts to heal yourself. And if you're ever in a dicey situation, the yellow explosive circles might allow you to "
                              "navigate your way past the obstacles with a two-time use double jump! Good luck ninja, I'll talk to you soon.\n\n"
                              "----------------------------------------------------------------\n\n";
const char* FAILIURE_MESSAGE = "That was a good attempt, but the Evil King got you. The Goddess managed to save you though, so try again!\n\n"
                               "----------------------------------------------------------------\n\n";
const char* PAUSE_MESSAGE = "Hey, the Goddess froze time so you could do whatever you need to do!\n\n"
                            "----------------------------------------------------------------\n\n";
const char* PORTAL_SENSED_MESSAGE = "The Goddess can sense the aura of the portal, we're more than halfway there!\n\n"
                                    "----------------------------------------------------------------\n\n";
const char* PORTAL_SEEN_MESSAGE = "That's the portal right there!\n\n"
                                  "----------------------------------------------------------------\n\n";
const char* VICTORY_MESSAGE = "Thank you for helping the ninja climb to the top of the tower and enter the portal! Unforutnately interdimensional travel is rather slow "
                              "but once he arrives in the REALM OF EVIL we will call on you once more. Until then, if you have the time, we request that "
                              "you help more ninjas like this poor soul climb the tower. After all, the more heroes we can send to the REALM OF EVIL, the better!\n\n"
                            "----------------------------------------------------------------\n\n";

// Game constants
const size_t NUM_LEVELS = 3;
const vector_t GRAVITY = {0, -1000};
const size_t BODY_ASSETS = 3;
const double BACKGROUND_CORNER = 150;
const double VERTICAL_OFFSET = 100;
const size_t ONE_HEART = 1;

typedef enum { USER, LEFT_WALL, RIGHT_WALL, PLATFORM, JUMP_POWER, 
              HEALTH_POWER, GHOST, GAS, PORTAL, QUICKSAND_ISLAND,
              SPIKE1, SPIKE2, SPIKE3, NONE } body_type_t;

typedef enum { GAME_START, GAME_RUNNING, GAME_PAUSED, 
              GAME_OVER, GAME_VICTORY } game_state_t;

typedef enum { GHOST_IMPACT, WIND, GAS_IMPACT, 
              PLATFORM_IMPACT, WALL_IMPACT } sound_type_t;

typedef struct sound {
  Mix_Chunk *player;
  sound_type_t *info;
} sound_t;

struct state {
  scene_t *scene;
  list_t *body_assets;
  asset_t *background_asset;
  asset_t *user_sprite;
  body_t *user;
  
  size_t user_health;
  asset_t *health_bar;
  double user_immunity;

  size_t ghost_counter;
  double ghost_timer;
  double velocity_timer;
  double restart_buffer;

  double vertical_offset;
  
  bool jumping; // determines whether up button can be pressed
  body_t *collided_obj; // the object that the user is collided with
  
  size_t jump_powerup_jumps;
  size_t jump_powerup_index;
  size_t health_powerup_index;

  asset_t *start_button;
  asset_t *game_title;
  asset_t *victory_background;
  asset_t *victory_text;
  asset_t *pause_button;
  asset_t *reset_button;
  game_state_t game_state;

  bool state_msg_tracker;
  bool distance_halfpoint;
  bool distance_portal;

  list_t *sounds;
  Mix_Music *music;
  double colliding_buffer;

  list_t *spikes;
  size_t spike2_idx;
  size_t spike3_idx;
};


/**
 * Frees all the memory allocated by a sound
 * sound_t struct
 * 
 * @param sound the pointer to the sound type
*/
void sound_free(sound_t *sound){
  Mix_FreeChunk(sound->player);
  free(sound->info);
  free(sound);
}

/**
 * Initializes all of the sound paths and 
 * stores them inside of the list sounds field
 * in the state
 * 
 * @param state the pointer to the state
*/
void sound_init(state_t *state){
  list_t *sounds = list_init(SOUND_SIZE, (free_func_t) sound_free);
  const char* paths[] = {GHOST_HIT_PATH, WIND_PATH, GAS_IMPACT_PATH, 
                        PLATFORM_IMPACT_PATH, WALL_IMPACT_PATH};
  for (size_t i = 0; i < SOUND_SIZE; i++){
    sound_t *sound = malloc(sizeof(sound_t));
    sound_type_t *info = malloc(sizeof(sound_type_t));
    assert(sound);
    assert(info);
    *info = i;
    sound->info = info;
    sound->player = sdl_load_sound(paths[i]);
    list_add(sounds, sound);
  }
  state->sounds = sounds;
}

/**
 * Gets the actual sound file from the state from
 * the type passed in
 * 
 * @param state the pointer to the state
 * @param sound_type the type of sound to be played
*/
Mix_Chunk *get_sound(state_t *state, sound_type_t sound_type){
  list_t* sounds = state->sounds;
  for (size_t i = 0; i < SOUND_SIZE; i++){
    sound_t *sound = list_get(sounds, i);
    if (*(sound->info) == sound_type){
      return sound->player;
    }
  }
  return NULL;
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
 * Convert body type into pointer
 * 
 * @param type body_type_t to be converted
 * @return body_type_t* pointer to type
*/
body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  assert(info);
  *info = type;
  return info;
}

/**
 * Creates a circle shape
 * @param ceneter the coordinates of the center of the circle
 * @param info a pointer to the type of the body
 * @param idx index of the body to be made. If multiple bodies 
 * are created in a loop, then the index distinguishes the coordinates
 * of the different bodies
 * 
 * @return list_t containing the points of the shape
*/
list_t *make_circle(vector_t center, body_type_t *info, size_t idx) {
  double radius = RADIUS;
  vector_t center_body = center;
  if (*info == GAS){
    radius = GAS_RADIUS;
    double y = (WALL_LENGTH.y/2) * (idx+1) - GAS_OFFSET;
    size_t position = idx % (GAS_NUM / NUM_LEVELS);
    double x = WALL_WIDTH.x + GAS_RADIUS * pow((-1), position + 1)
             + GAP_DISTANCE * (1 - position);
    center_body = (vector_t){x, y};
  } else if (*info == PORTAL){
    radius = PORTAL_RADIUS;
    double y = WALL_LENGTH.y * NUM_LEVELS + PORTAL_OFFSET;
    double x = GAP_DISTANCE / 2 + WALL_WIDTH.x;
    center_body = (vector_t){x, y};
  } else if (*info == SPIKE1 || *info == SPIKE2 || *info == SPIKE3){
    radius = SPIKE_RADIUS;
    double y = (WALL_LENGTH.y) * (idx+1) - SPIKE_OFFSET;
    double x = WALL_WIDTH.x - SPIKE_RADIUS 
              + GAP_DISTANCE;
    center_body = (vector_t){x, y};
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
 * Generates the list of points for a Wall shape
 * given the vector of the bottom left
 * corner
 *
 * @param corner a vector that contains the coordinates 
 * of the bottom left corner of
 * the wall
 * @param points an empty list to add the points to, 
 * the points are pointers to vectors
 */
void make_rectangle_points(vector_t corner, list_t *points, body_type_t *info){
  vector_t gap = {MAX.x, 0};
  vector_t temp[] = {ISLAND_LENGTH, gap, vec_negate(ISLAND_LENGTH)};
  if (*info == PLATFORM){
    temp[0] = PLATFORM_LENGTH;
    temp[1] = PLATFORM_WIDTH;
    temp[2] = vec_negate(PLATFORM_LENGTH);
    
  }
  if (*info == LEFT_WALL || *info == RIGHT_WALL){
    temp[0] = WALL_LENGTH;
    temp[1] = WALL_WIDTH;
    temp[2] = vec_negate(WALL_LENGTH);
  }
  vector_t *v_1 = malloc(sizeof(*v_1));
  assert(v_1);
  *v_1 = corner;
  list_add(points, v_1);
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
 * @param level the level at which to play the object
*/
list_t *make_rectangle(body_type_t *wall_info, size_t level) {
  vector_t corner = VEC_ZERO;
  body_type_t *info = wall_info;

  if (*info == LEFT_WALL) {
    corner = (vector_t){MIN.x, MIN.y + WALL_LENGTH.y * level};
  } else if (*info == RIGHT_WALL) {
    corner = (vector_t){MAX.x - WALL_WIDTH.x, MIN.y + WALL_LENGTH.y * level};
  } else if (*info == PLATFORM) {
    double x_offset = 0;
    size_t middle = 0;
    if (level > 0){
      x_offset = GAP_DISTANCE/2;
      middle = MIDDLE;
    }
    corner = (vector_t){MIN.x + WALL_WIDTH.x + x_offset - 
                        PLATFORM_WIDTH.x/2 * middle, PLATFORM_HEIGHT + 
                        level * WALL_LENGTH.y/2};
  } else if (*info == QUICKSAND_ISLAND) {
    corner = (vector_t){MIN.x, MIN.y - ISLAND_LENGTH.y};
  }
  list_t *c = list_init(WALL_POINTS, free);
  make_rectangle_points(corner, c, info);
  return c;
}

/**
 * Generates the list of points for a powerup shape
 * given the size of the powerup and
 * the relative location in the vertical direction.
 *
 * @param length corresponds to the length/width of the generated powerup
 * @param power_up_y_loc the relative location of the 
 * powerup in the y direction
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
 * Initializes the user at the beginning of the game
 * @param state pointer to the current state of the game
 */
void create_user(state_t *state) {
  vector_t center = {MIN.x + RADIUS + WALL_WIDTH.x, 
                    MIN.y + RADIUS + PLATFORM_HEIGHT + PLATFORM_LENGTH.y};
  list_t *points = make_circle(center,  make_type_info(USER), ZERO_SEED);
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
  scene_t *scene = state->scene;
  body_type_t *info = NULL;
  for (size_t i = 0; i < NUM_LEVELS; i++){
    for (size_t j = 0; j < WALL_TYPES; j++){
      if (j == 0){
        info = make_type_info(LEFT_WALL);
      } else {
        info = make_type_info(RIGHT_WALL);
      }
      list_t *points = make_rectangle(info, i);
      body_t *wall = body_init_with_info(points, WALL_MASS, 
                                              USER_COLOR, info, 
                                              NULL);
      scene_add_body(scene, wall);
      asset_t *wall_asset = asset_make_image_with_body(WALL_PATH, wall, 
                                                      VERTICAL_OFFSET);
      list_add(state->body_assets, wall_asset);
    }
  }

  for (size_t i = 0; i < NUM_PLATFORMS; i++){
    list_t *platform_points = make_rectangle(make_type_info(PLATFORM), i);
    body_t *platform = body_init_with_info(platform_points, WALL_MASS, 
                                              USER_COLOR, make_type_info(PLATFORM), 
                                              NULL);
    scene_add_body(scene, platform);
    asset_t *wall_asset_platform = asset_make_image_with_body(PLATFORM_PATH, 
                                                              platform, 
                                                              VERTICAL_OFFSET);
    list_add(state->body_assets, wall_asset_platform);
    state->collided_obj = platform; // inital start location
  }

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
void health_powerup_collision(body_t *body1, body_t *body2, vector_t axis, 
                              void *aux, double force_const) {
  state_t *state = aux;
  if (state->user_health < FULL_HEALTH) {
      body_remove(body2);
      list_remove(state->body_assets, state->health_powerup_index);
      state->user_health++;
  } else {
    return;
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
void jump_powerup_collision(body_t *body1, body_t *body2, vector_t axis, 
                            void *aux, double force_const) {
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
 * Creates a jump power up and adds to state
 * @param state the current state of the demo
*/
void create_jump_power_up(state_t *state) {
  list_t *points = make_power_up_shape(POWERUP_LENGTH, JUMP_POWERUP_LOC);
  body_t *powerup = body_init_with_info(points, POWERUP_MASS, USER_COLOR, 
                                        make_type_info(JUMP_POWER), NULL);
  asset_t *powerup_asset = asset_make_image_with_body(JUMP_POWERUP_PATH, 
                                                      powerup, 
                                                      state->vertical_offset);
  state->jump_powerup_index = list_size(state->body_assets);
  state->jump_powerup_jumps = 0;
  list_add(state->body_assets, powerup_asset);
  scene_add_body(state->scene, powerup);
  create_collision(state->scene, state->user, powerup,
                  (collision_handler_t)jump_powerup_collision, 
                        state, POWERUP_ELASTICITY);
}

/**
 * Creates a health power up and adds to state
 * @param state the current state of the demo
*/
void create_health_power_up(state_t *state) {
  list_t *points = make_power_up_shape(POWERUP_LENGTH, HEALTH_POWERUP_LOC);
  body_t *powerup = body_init_with_info(points, POWERUP_MASS, USER_COLOR, 
                                        make_type_info(HEALTH_POWER), NULL);
  asset_t *powerup_asset = asset_make_image_with_body(HEALTH_POWERUP_PATH, 
                                                      powerup, 
                                                      state->vertical_offset);
  state->health_powerup_index = list_size(state->body_assets);
  list_add(state->body_assets, powerup_asset);
  scene_add_body(state->scene, powerup);
  create_collision(state->scene, state->user, powerup,
                  (collision_handler_t)health_powerup_collision, 
                  state, POWERUP_ELASTICITY);
}

/**
 * Creates a portal and adds to state
 * @param state the current state of the demo
*/
void create_portal(state_t *state) {
  list_t *points = make_circle(VEC_ZERO, make_type_info(PORTAL), ZERO_SEED);
  body_t *portal = body_init_with_info(points, PORTAL_MASS, USER_COLOR, 
                                        make_type_info(PORTAL), NULL);
  asset_t *portal_asset = asset_make_image_with_body(PORTAL_PATH, portal, 
                                                    state->vertical_offset);
  list_add(state->body_assets, portal_asset);
  scene_add_body(state->scene, portal);
}

/**
 * Creates an island and adds to state
 * @param state the current state of the demo
*/
void create_island(state_t *state) {
  list_t *points = make_rectangle(make_type_info(QUICKSAND_ISLAND), ISLAND_LEVEL);
  body_t *island = body_init_with_info(points, ISLAND_MASS, USER_COLOR, 
                                        make_type_info(QUICKSAND_ISLAND), NULL);
  asset_t *island_asset = asset_make_image_with_body(ISLAND_PATH, island, 
                                                    state->vertical_offset);
  list_add(state->body_assets, island_asset);
  scene_add_body(state->scene, island);
}

/**
 * Called whenever the user health changes so that the health bar asset can be updated
 * 
 * @param state state object representing the current demo state
*/
void update_health_bar(state_t *state) {
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, HEALTH_BAR_BOX);
  
  if (state->user_health == 2) {
    health_bar_asset = asset_make_image(HEALTH_BAR_2_PATH, HEALTH_BAR_BOX);
  } else if (state->user_health == 1) {
    health_bar_asset = asset_make_image(HEALTH_BAR_1_PATH, HEALTH_BAR_BOX);
  } else if (state->user_health == 0) {
    health_bar_asset = asset_make_image(HEALTH_BAR_0_PATH, HEALTH_BAR_BOX);
  }

  state->health_bar = health_bar_asset;
}

/**
 * Check whether two bodies are colliding and 
 * applies a sticky collision between them
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
  if (state->colliding_buffer > COLLIDING_BUFFER){
    Mix_PlayChannel(IMPACT_CHANNEL, get_sound(state, PLATFORM_IMPACT), 0);
  }

  state->colliding_buffer = 0;
}


/**
 * Check whether two bodies are colliding and applies a 
 * damaging collision between them
 *
 * @param state state object representing the current demo state
 * @param user the user
 * @param body the body with which the user is colliding
 * @param aux information about the state of the collision
 * @param force_const the force constant to be applied to the collision
 */
void damaging_collision(body_t *user, body_t *body, vector_t axis, void *aux,
                double force_const){
  state_t *state = aux;
  if (state->user_immunity > IMMUNITY){
    if (state->user_health >= ONE_HEART){
      state->user_health --;
      sdl_play_sound(get_sound(state, GHOST_IMPACT));
    }
    state->user_immunity = 0;
  }
}



/**
 * Check whether two bodies are colliding and applies a 
 * explosive collision between them
 *
 * @param state state object representing the current demo state
 * @param user the user
 * @param spike the body with which the user is colliding
 * @param aux information about the state of the collision
 * @param force_const the force constant to be applied to the collision
 */
void spike_collision(body_t *user, body_t *spike, vector_t axis, void *aux,
                double force_const){
  state_t *state = aux;
  if (state->user_health >= ONE_HEART){
    state->user_health --;
    sdl_play_sound(get_sound(state, GHOST_IMPACT));
  }
  switch (get_type(spike)) {
    case SPIKE1:{
      list_remove(state->spikes, SPIKE1_INDEX);
      state->spike2_idx --;
      state->spike3_idx --;
      break;
    }
    case SPIKE2:{
      list_remove(state->spikes, state->spike2_idx);
      state->spike3_idx --;
    }
      break;
    case SPIKE3:
      list_remove(state->spikes, state->spike3_idx);
      break;
    default:
      break;
    }
  body_remove(spike);
  state->user_immunity = 0;
}

/**
 * Creates a spikes and adds to state
 * @param state the current state of the demo
*/
void create_spikes(state_t *state) {
  for (size_t i = 0; i < NUM_SPIKES; i++){
    list_t *points = make_circle(VEC_ZERO, make_type_info(SPIKE1_ENUM + i), i);
    body_t *spike = body_init_with_info(points, SPIKE_MASS, USER_COLOR, 
                                          make_type_info(SPIKE1_ENUM + i), NULL);
    asset_t *spike_asset = asset_make_image_with_body(SPIKE_PATH, spike, 
                                                      state->vertical_offset);
    list_add(state->spikes, spike_asset);
    create_collision(state->scene, state->user, spike,
                      (collision_handler_t)spike_collision, state, 
                      SPIKE_ELASTICITY);
    scene_add_body(state->scene, spike);
  }
}

/**
 * Check whether two bodies are colliding and applies a collision between
 * portal and body
 *
 * @param state state object representing the current demo state
 * @param user a pointer to the body of the user
 * @param portal a pointer to the body of the portal
 */
void portal_collision(body_t *user, body_t *portal, vector_t axis, void *aux,
                double force_const){
  state_t *state = aux;
  state->game_state = GAME_VICTORY;
}

/**
 * Spawns a ghost on the screen at fixed y value and at a random x value
 * that is within the bounds of the window
 * @param state pointer to the current demo state
 */
void spawn_ghost(state_t *state) {
  vector_t max = {MAX.x, VEC_ZERO.y};
  double x = rand_vec(VEC_ZERO, max, ZERO_SEED).x;
  vector_t ghost_center = {x, Y_OFFSET_GHOST};
  list_t *c = make_circle(ghost_center, make_type_info(GHOST), ZERO_SEED);
  body_t *ghost = body_init_with_info(c, GHOST_MASS, GHOST_COLOUR, 
                                      make_type_info(GHOST), NULL);
  scene_add_body(state->scene, ghost);
  asset_t *ghost_asset = asset_make_image_with_body(GHOST_PATH, ghost, 
                                                    VERTICAL_OFFSET);
  create_collision(state->scene, state->user, ghost,
                  (collision_handler_t)damaging_collision, 
                  state, GHOST_ELASTICITY);
  list_add(state->body_assets, ghost_asset);
  state->ghost_counter++;
  state->ghost_timer = 0;
}

/**
 * Spawns a ghost on the screen at fixed y value and at a random x value
 * that is within the bounds of the window
 * @param state pointer to the current state of the game
 */
void ghost_move(state_t *state){
  scene_t *scene = state->scene;
  size_t num_bodies = scene_bodies(scene);
  for (size_t i = 0; i < num_bodies; i++){
    body_t *body = scene_get_body(scene, i);
    if (get_type(body) == GHOST && 
    (state->velocity_timer > VELOCITY_BUFFER)){
      vector_t user_center = body_get_centroid(state->user);
      vector_t ghost_center = body_get_centroid(body);
      vector_t direction = vec_unit(vec_add(user_center, 
                                    vec_negate(ghost_center)));
      vector_t velocity = vec_multiply(GHOST_SPEED, direction);
      vector_t rand_add = rand_vec(vec_negate(RAND_VELOCITY), 
                                  RAND_VELOCITY, i);
      vector_t rand_velocity = vec_add(velocity, rand_add);
      body_set_velocity(body, rand_velocity);
      if (i == num_bodies - 1){
        state->velocity_timer = 0;
      }
    }
  }
}


/**
 * Spawns toxic gas on the screen at fixed positions
 * that is within the bounds wall height and the space in between
 */
void spawn_gas(state_t *state) {
  for (size_t i = 0; i < GAS_NUM; i++){
    list_t *c = make_circle(VEC_ZERO, make_type_info(GAS), i);
    body_t *gas = body_init_with_info(c, GAS_MASS, GHOST_COLOUR, 
                                        make_type_info(GAS), NULL);
    scene_add_body(state->scene, gas);
    asset_t *gas_asset = asset_make_image_with_body(GAS_PATH, gas, 
                                                    VERTICAL_OFFSET);
    list_add(state->body_assets, gas_asset);
  }
}

/** 
 * spawn and move ghosts during the game
 * 
 * @param state the current demo of the state
*/
void spawn_and_move_ghosts(state_t *state) {
  if (state->ghost_timer > SPAWN_TIMER && 
      state->ghost_counter <= GHOST_NUM){
    spawn_ghost(state);
  }

  if (state->restart_buffer > RESTART_BUFFER){
    ghost_move(state);
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
  } else {
    body_reset(state->user);
    bool is_collided = false;
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
  vector_t v1 = body_get_velocity(state->user);

  if (get_type(state->collided_obj) == PLATFORM) {
    body_set_velocity(state->user, (vector_t) {v1.x * PLATFORM_FRICTION, 0});
  } else if (get_type(state->collided_obj) == LEFT_WALL || 
             get_type(state->collided_obj) == RIGHT_WALL) {
      body_add_force(state->user, WALL_FRICTION_FORCE);
  }
}

/**
 * Handler for the start button
 * @param state pointer to the current state
 */
void start_button_handler(state_t *state) {
  state->game_state = GAME_RUNNING;
  state->restart_buffer = 0;
}

/**
 * Handler for the pause button
 * @param state pointer to the current state
 */
void pause_button_handler(state_t *state) {
  if (state->game_state == GAME_RUNNING) {
    state->game_state = GAME_PAUSED;
  } else if (state->game_state == GAME_PAUSED) {
    state->game_state = GAME_RUNNING;
  }
}

/**
 * Handler for the restart button
 * @param state pointer to the current state
 */
void reset_button_handler(state_t *state) {
  state->user_health = FULL_HEALTH;
  state->game_state = GAME_RUNNING;
  state->restart_buffer = 0;
  state->distance_halfpoint = false;


  bool contains_jump = false;
  bool contains_health = false;

  // Update user and ghost centers
  vector_t user_center = {MIN.x + RADIUS + WALL_WIDTH.x, 
                    MIN.y + RADIUS + PLATFORM_HEIGHT + PLATFORM_LENGTH.y};
  body_set_centroid(state->user, user_center);

  // Update ghost centers
  size_t num_bodies = scene_bodies(state->scene);
  vector_t max = {MAX.x, 0};
  
  for (size_t i = 0; i < num_bodies; i++){
    body_t *body = scene_get_body(state->scene, i);
    if (get_type(body) == GHOST){
      double x = rand_vec(VEC_ZERO, max, ZERO_SEED).x;
      vector_t ghost_center = {x, Y_OFFSET_GHOST};
      body_set_centroid(body, ghost_center);
      body_set_velocity(body, VEC_ZERO);
    }
    if (get_type(body) == JUMP_POWER){
      contains_jump = true;
    }
    if (get_type(body) == HEALTH_POWER){
      contains_health = true;
    }
    if (get_type(body) == SPIKE1 || get_type(body) == SPIKE2 ||
        get_type(body) == SPIKE3){
      body_remove(body);
    }
  }
  if (!contains_jump){
    create_jump_power_up(state);
    
  }
  if (!contains_health){
    create_health_power_up(state);
  }
  state->spikes = list_init(NUM_SPIKES, (free_func_t) asset_destroy);
  state->spike2_idx = SPIKE2_INDEX;
  state->spike3_idx = SPIKE3_INDEX;
  create_spikes(state);
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
  vector_t cur_pos = body_get_centroid(user);
  double new_vx = cur_v.x;
  double new_vy = cur_v.y;


  if (type == KEY_PRESSED) {
    switch (key) {
      case LEFT_ARROW: {
        if (cur_pos.x > WALL_WIDTH.x + WALL_BUFFER) {
          new_vx = -1 * (RESTING_SPEED + ACCEL * held_time);
        }
        break;
      }
      case RIGHT_ARROW: {
        if (cur_pos.x < MAX.x - WALL_WIDTH.x - WALL_BUFFER) {
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
                      (collision_handler_t)sticky_collision, state, ELASTICITY);
      break;
    case RIGHT_WALL:
      create_collision(state->scene, state->user, body,
                      (collision_handler_t)sticky_collision, state, ELASTICITY);
      break;
    case PLATFORM:
      create_collision(state->scene, state->user, body,
                      (collision_handler_t)sticky_collision, 
                      state, ELASTICITY);
      break;
    case GAS:
      create_collision(state->scene, state->user, body, 
                      (collision_handler_t)damaging_collision, 
                      state, ELASTICITY);
      break;
    case PORTAL:
      create_collision(state->scene, state->user, body, 
                      (collision_handler_t)portal_collision, state, ELASTICITY);
      break;
    case QUICKSAND_ISLAND:
      create_collision(state->scene, state->user, body, 
                      (collision_handler_t)sticky_collision, state, ELASTICITY);
      break;
    default:
      break;
    }
  }
}

/**
 * Updates all the buffers in the main loop
 * 
 * @param dt the time between each tick
 * @param state the current demo state
 */
void update_buffers(state_t *state, double dt){
  state->ghost_timer += dt;
  state->velocity_timer += dt;
  state->user_immunity += dt;
  state->colliding_buffer += dt;
  state->restart_buffer += dt;
}

/**
 * Prints out the relevant portion of the storyline for game based on game_state.
 * 
 * @param state the current demo state
*/
void print_story(state_t *state) {
  if (state->game_state == GAME_START && 
      state->state_msg_tracker == false) {
    printf("%s", WELCOME_MESSAGE);
    state->state_msg_tracker = true;
  } else if (state->game_state == GAME_OVER && 
             state->state_msg_tracker == false) {
    printf("%s", FAILIURE_MESSAGE);
    state->state_msg_tracker = true;
  } else if (state->game_state == GAME_PAUSED && 
             state->state_msg_tracker == false) {
    printf("%s", PAUSE_MESSAGE);
    state->state_msg_tracker = true;
  } else if (state->game_state == GAME_VICTORY && 
             state->state_msg_tracker == false) {
    printf("%s", VICTORY_MESSAGE);
    state->state_msg_tracker = true;
  } else if (state->game_state == GAME_RUNNING) {
    state->state_msg_tracker = false;
  }

  if (state->vertical_offset >= HALFWAY_VERTICAL_DISTANCE && 
      state->distance_halfpoint == false && 
      state->game_state == GAME_RUNNING && 
      state->restart_buffer >= RESTART_BUFFER) {
    printf("%s", PORTAL_SENSED_MESSAGE);
    state->distance_halfpoint = true;
  } else if (state->vertical_offset >= PORTAL_VERTICAL_DISTANCE &&
             state->distance_portal == false && 
             state->game_state == GAME_RUNNING &&
             state->restart_buffer >= RESTART_BUFFER) {
    printf("%s", PORTAL_SEEN_MESSAGE);
    state->distance_portal = true;
  }
}

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  asset_cache_init();
  state_t *state = malloc(sizeof(state_t));
  assert(state);

  // Initialize scene
  state->scene = scene_init();
  state->body_assets = list_init(BODY_ASSETS, (free_func_t)asset_destroy);

  // Initialize sound and music
  Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, STEREO, AUDIO_BUFFER);
  Mix_Volume(DEFAULT_CHANNEL, MIX_MAX_VOLUME/2);
  sound_init(state);
  state->colliding_buffer = 0;
  Mix_PlayChannel(WIND_CHANNEL, get_sound(state, WIND), LOOPS);
  state->music = Mix_LoadMUS(MUSIC_PATH);
  
  // Initialize backgrounds
  SDL_Rect background_box = {.x = MIN.x, .y = MIN.y, 
                            .w = MAX.x, .h = MAX.y};
  state->background_asset = asset_make_image(BACKGROUND_PATH, 
                                              background_box);
  SDL_Rect victory_background_box = {.x = MIN.x, .y = MIN.y / 2, 
                                    .w = MAX.x, .h = MAX.y / 2};
  state->victory_background = asset_make_image(VICTORY_BACKGROUND_PATH, 
                                                victory_background_box);

  // Initialize health bar
  asset_t *health_bar_asset = asset_make_image(FULL_HEALTH_BAR_PATH, 
                                              HEALTH_BAR_BOX);
  state->health_bar = health_bar_asset;

  // Initialize user
  create_user(state);
  asset_t *user_asset = asset_make_image_with_body(USER_PATH, state->user, 
                                                  state->vertical_offset);
  list_add(state->body_assets, user_asset);

  // Intialize walls and platforms
  create_walls_and_platforms(state);

  // Initialize powerups
  create_health_power_up(state);
  create_jump_power_up(state);

  // Initialize obstacles, portal and island
  spawn_gas(state);
  create_portal(state);
  create_island(state);
  

  // Initialize spike idx
  state->spikes = list_init(NUM_SPIKES, (free_func_t) asset_destroy);
  state->spike2_idx = SPIKE2_INDEX;
  state->spike3_idx = SPIKE3_INDEX;
  create_spikes(state);

  // Initialize buttons and in-game text
  SDL_Rect game_title_box = {.x = MAX.x / 2 - 250, .y = TITLE_OFFSETS.y, 
                            .w = 500, .h = 100};
  state->game_title = asset_make_image(TITLE_PATH, game_title_box);

  SDL_Rect victory_text_box = {.x = MAX.x / 2 - 200, .y = TITLE_OFFSETS.y, 
                              .w = 400, .h = 200};
  state->victory_text = asset_make_image(VICTORY_TEXT_PATH, victory_text_box);

  SDL_Rect button_box = {.x = MAX.x / 2 - 100, .y = BUTTON_OFFSETS.y, 
                        .w = 200, .h = 100};

  state->start_button = asset_make_button(button_box, 
                        asset_make_image(START_BUTTON_PATH, button_box),
                        NULL, (button_handler_t)start_button_handler);
  asset_cache_register_button(state->start_button);

  state->reset_button = asset_make_button(button_box, 
                        asset_make_image(RESET_BUTTON_PATH, button_box), 
                        NULL, (button_handler_t)reset_button_handler);
  asset_cache_register_button(state->reset_button);

  SDL_Rect pause_button_box = {.x = MAX.x - PAUSE_BUTTON_OFFSETS.x, 
                              .y = PAUSE_BUTTON_OFFSETS.y, .w = 35, .h = 30};
  state->pause_button = asset_make_button(pause_button_box, 
                        asset_make_image(PAUSE_BUTTON_PATH, pause_button_box), 
                        NULL, (button_handler_t)pause_button_handler);
  asset_cache_register_button(state->pause_button);

  // Initialize miscellaneous state values
  state->game_state = GAME_START;
  state->state_msg_tracker = false;
  state->distance_halfpoint = false;
  state->distance_portal = false;
  state->vertical_offset = 0;
  state->velocity_timer = 0;
  state->ghost_counter = 0;
  state->user_immunity = 0;
  state->ghost_timer = 0;
  state->restart_buffer = 0;
  
  add_force_creators(state);
  sdl_on_key((key_handler_t)on_key);

  return state;
}

bool emscripten_main(state_t *state) {
  print_story(state);

  double dt = time_since_last_tick();
  update_buffers(state, dt);
  
  body_t *user = state->user;
  scene_t *scene = state->scene;

  if (state->game_state == GAME_RUNNING) {
    scene_tick(scene, dt);
    body_tick(user, dt);
  }

  sdl_clear();

  check_gravity_and_friction(state);

  vector_t player_pos = body_get_centroid(user);
  state->vertical_offset = player_pos.y - VERTICAL_OFFSET;

  spawn_and_move_ghosts(state);
  
  // Render assets
  asset_render(state->background_asset, state->vertical_offset);
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i), state->vertical_offset);
  }
  for (size_t i = 0; i < list_size(state->spikes); i++) {
    asset_render(list_get(state->spikes, i), state->vertical_offset);
  }
  update_health_bar(state);
  asset_render(state->health_bar, state->vertical_offset);

  // Render buttons and/or title based on game state
  if (state->game_state == GAME_START) {
    asset_render(state->game_title, state->vertical_offset);
    asset_render(state->start_button, state->vertical_offset);
  } else if (state->game_state == GAME_RUNNING) {
    asset_render(state->pause_button, state->vertical_offset);
  } else if (state->game_state == GAME_PAUSED) {
    asset_render(state->pause_button, state->vertical_offset);
    asset_render(state->reset_button, state->vertical_offset);
  } else if (state->game_state == GAME_OVER) {
    asset_render(state->reset_button, state->vertical_offset);
  } else if (state->game_state == GAME_VICTORY) {
    state->distance_halfpoint = false;
    state->distance_portal = false;
    asset_render(state->victory_background, state->vertical_offset);
    asset_render(state->reset_button, state->vertical_offset);
    asset_render(state->victory_text, state->vertical_offset);
  }

  if (Mix_PlayingMusic() == 0) {
    Mix_PlayMusic(state->music, -1);
    Mix_VolumeMusic(MUSIC_VOLUME);
  }

  sdl_show(state->vertical_offset);

  if (state->user_health == 0) {
    state->game_state = GAME_OVER;
  }

  return false;
}

void emscripten_free(state_t *state) {
  TTF_Quit();
  list_free(state->sounds);
  Mix_FreeMusic(state->music);
  scene_free(state->scene);
  list_free(state->body_assets);
  list_free(state->spikes);
  body_free(state->user);
  asset_cache_destroy();
  free(state);
}
