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
const vector_t MAX = {1000, 500};

// User constants
const double USER_MASS = 5;
const rgb_color_t USER_COLOR = (rgb_color_t){0, 0, 0};
const char *USER_INFO = "user";
const double USER_ROTATION = 0;
const vector_t USER_CENTER = {500, 60};  // Change so that the user is stuck
                                        // onto a wall to begin game

const double OUTER_RADIUS = 60;
const double INNER_RADIUS = 15;
const size_t USER_NUM_POINTS = 20;


struct state {
  scene_t *scene;
  list_t *body_assets;
  body_t *user_body;
  size_t *user_health;
  size_t ghost_counter;
  double ghost_timer;
  bool game_over;
};

list_t *make_user(double outer_radius, double inner_radius) {
  vector_t center = USER_CENTER;
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
 * Check conditions to see if game is over. Game is over if the user has no more health
 * (loss), the user falls off the map (loss)
 * or the user reaches the top of the map (win).
 *
 * @param state a pointer to a state object representing the current demo state
 */
bool game_over(state_t *state) {
  return false;
}


state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  assert(state);
  state->scene = scene_init();
  list_t *points = make_user(OUTER_RADIUS, INNER_RADIUS);
  state->user_body =
      body_init_with_info(points, USER_MASS, USER_COLOR, (void *)USER_INFO, NULL);
  body_set_rotation(state->user_body, USER_ROTATION);
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
