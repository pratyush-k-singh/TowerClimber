#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {700, 500};

// User constants
const double USER_MASS = 5;
const rgb_color_t USER_COLOR = (rgb_color_t){0, 0, 0};
const char *USER_INFO = "user";
const double USER_ROTATION = 0;
const vector_t USER_CENTER = {500, 60}; //(HERE JUST IN CASE NEED TO USE)
const double OUTER_RADIUS = 60;
const double INNER_RADIUS = 15;
const size_t USER_NUM_POINTS = 20;
const double GAP = 10;

// Wall constants
const vector_t WALL_WIDTH = {50, 0};
const size_t WALL_POINTS = 4;
const double WALL_MASS = INFINITY;
const double WALL_ELASTICITY = 0.1;

const char *LEFT_WALL_INFO = "left_wall";
const char *RIGHT_WALL_INFO = "right_wall";

// Game constants
const size_t NUM_LEVELS = 1;


struct state {
  scene_t *scene;
  list_t *body_assets;
  body_t *user_body;
  size_t user_health;
  size_t ghost_counter;
  double ghost_timer;
  bool game_over;
};

list_t *make_user(double outer_radius, double inner_radius) {
  vector_t center = {MIN.x + inner_radius + WALL_WIDTH.x + GAP, 
                    MIN.y + outer_radius};
  center.y += inner_radius;
  list_t *c = list_init(USER_NUM_POINTS, free);
  for (size_t i = 0; i < USER_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / USER_NUM_POINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){center.x + inner_radius * cos(angle),
                    center.y + outer_radius * sin(angle)};
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
  vector_t wall_length = {0, MAX.y};
  vector_t *v_1 = malloc(sizeof(*v_1));
  *v_1 = corner;
  vector_t *v_2 = malloc(sizeof(*v_2));
  *v_2 = vec_add(*v_1, wall_length);
  vector_t *v_3 = malloc(sizeof(*v_3));
  *v_3 = vec_add(*v_2, WALL_WIDTH);
  vector_t *v_4 = malloc(sizeof(*v_4));
  *v_4 = vec_subtract(*v_3, wall_length);
  list_add(points, v_1);
  list_add(points, v_2);
  list_add(points, v_3);
  list_add(points, v_4);
}

list_t *make_wall(void *wall_info) {
  vector_t corner = VEC_ZERO;
  vector_t left_wall_corner = MIN;
  vector_t right_wall_corner = {MAX.x - 50, 0};
  if (strcmp(wall_info, LEFT_WALL_INFO) == 0){
    corner = left_wall_corner;
  } else {
    corner = right_wall_corner;
  }
  list_t *c = list_init(WALL_POINTS, free);
  make_wall_points(corner, c);
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
  return false;
}

// initialize the walls at start of game
void wall_init(state_t *state) {
  scene_t *scene = state -> scene;
  for (size_t i = 0; i < NUM_LEVELS; i++){
    list_t *left_points = make_wall((void *)LEFT_WALL_INFO);
    list_t *right_points = make_wall((void *)RIGHT_WALL_INFO);
    body_t *left_wall = body_init_with_info(left_points, 3, 
                                            USER_COLOR, (void *)LEFT_WALL_INFO, 
                                            NULL);
    body_t *right_wall = body_init_with_info(right_points, WALL_MASS, 
                                            USER_COLOR, (void *)RIGHT_WALL_INFO, 
                                            NULL);
    scene_add_body(scene, left_wall);
    scene_add_body(scene, right_wall);
    create_destructive_collision(scene, state -> user_body, left_wall);
    // create_collision(scene, left_wall, state -> user_body, 
    //                 create_destructive_collision, NULL, WALL_ELASTICITY);
  }
}


state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  assert(state);
  state->scene = scene_init();
  wall_init(state);
  list_t *points = make_user(OUTER_RADIUS, INNER_RADIUS);
  state->user_body =
      body_init_with_info(points, USER_MASS, USER_COLOR, (void *)USER_INFO, NULL);
  body_set_rotation(state->user_body, USER_ROTATION);
 // body_set_velocity(state -> user_body, (vector_t){50, 10});
  state->game_over = false;
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  body_t *user = state->user_body;
  scene_t *scene = state->scene;

  scene_tick(scene, dt);
  sdl_render_scene(scene, user);
  body_tick(user, dt);

  return game_over(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  body_free(state->user_body);
  free(state);
}
