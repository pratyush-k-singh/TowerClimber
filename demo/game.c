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

const double USER_RADIUS = 15;
const double USER_MASS = 5;
const double CIRC_NPOINTS = 20;
const vector_t USER_INIT_POS = {500, 70};

const vector_t VEC_ZERO = {0, 0};
const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 1000};

rgb_color_t RED = (rgb_color_t){0.5, 0.5, 0.5};

typedef struct mouse_state mouse_state_t;

struct mouse_state {
  vector_t start_pos;
  vector_t end_pos;
  bool aiming;
};

struct state {
  scene_t *scene;
  body_t *user; 
  mouse_state_t *mouse;
};

list_t *make_circle(vector_t center, double radius) {
  list_t *c = list_init(CIRC_NPOINTS, free);
  for (size_t i = 0; i < CIRC_NPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_NPOINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(c, v);
  }
  return c;
}


state_t *emscripten_init() {
  sdl_init(MIN, MAX);

  state_t *state = malloc(sizeof(state_t));
  assert(state);

  state->scene = scene_init();

  list_t *user_shape = make_circle(USER_INIT_POS, USER_RADIUS);
  state->user = body_init_with_info(user_shape, USER_MASS, RED, NULL, NULL);
  scene_add_body(state->scene, state->user);

  state->mouse = malloc(sizeof(mouse_state_t));
  state->mouse->start_pos = VEC_ZERO;
  state->mouse->end_pos = VEC_ZERO;
  state->mouse->aiming = false;
  
  return state;
}

bool emscripten_main(state_t *state) {
  sdl_render_scene(state->scene, NULL);
  

  return false;
}

void emscripten_free(state_t *state) {
  body_free(state->user);
  free(state->mouse);
  free(state);
}
