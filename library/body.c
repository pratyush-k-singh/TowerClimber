#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"

const size_t INITIAL_SIZE = 10;
const double INITIAL_ROT = 0;

struct body {
  polygon_t *poly;

  double mass;

  vector_t force;
  vector_t impulse;
  bool removed;

  void *info;
  free_func_t info_freer;
};

void body_set_centroid(body_t *body, vector_t x);

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(mass > 0);
  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);

  body->poly =
      polygon_init(shape, VEC_ZERO, INITIAL_ROT, color.r, color.g, color.b);
  body_set_centroid(body, polygon_get_center(body->poly));

  body->mass = mass;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->removed = false;
  body->info = info;
  body->info_freer = info_freer;

  return body;
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  // Pass NULL for info and info_freer since they are not used here
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

void body_free(body_t *body) {
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  polygon_free(body->poly);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  body_get_rotation(body);
  polygon_t *polygon = body->poly;
  list_t *vec_list = list_init(INITIAL_SIZE, NULL);
  list_t *temp = polygon_get_points(polygon);

  for (size_t i = 0; i < list_size(temp); i++) {
    vector_t *point = list_get(temp, i);
    list_add(vec_list, point);
  }

  return vec_list;
}

vector_t body_get_centroid(body_t *body) {
  polygon_t *polygon = body->poly;
  vector_t centroid = polygon_get_center(polygon);
  return centroid;
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_t *polygon = body->poly;
  vector_t centroid = polygon_get_center(polygon);

  // Get translation vector
  vector_t translation = vec_subtract(x, centroid);

  // Translate every point
  polygon_translate(polygon, translation);
  polygon_set_center(polygon, x);
}

vector_t body_get_velocity(body_t *body) {
  vector_t velocity = polygon_get_velocity(body->poly);
  return velocity;
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_t *polygon = body->poly;
  double tot_angle = polygon_get_rotation(polygon);
  double rotation_angle = angle - tot_angle;

  vector_t center = polygon_get_center(polygon);
  polygon_rotate(polygon, rotation_angle, center);
  polygon_set_rotation(polygon, angle);
}

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

void body_tick(body_t *body, double dt) {
  vector_t v_init = body_get_velocity(body);
  vector_t force = body->force;
  vector_t impulse = body->impulse;
  double mass = body->mass;

  // calculate change in velocity due to impulse
  vector_t v_imp = vec_multiply(1 / mass, impulse);

  // calculate change in velocity due to force
  vector_t v_force = vec_multiply(1 / mass, vec_multiply(dt, force));

  // calculate new velocity
  vector_t v_tmp = vec_add(v_init, v_imp);
  vector_t v_new = vec_add(v_tmp, v_force);

  // find average of initial and calculated velocity
  double v_avg_x = (v_init.x + v_new.x) / 2;
  double v_avg_y = (v_init.y + v_new.y) / 2;
  vector_t v_avg = {v_avg_x, v_avg_y};

  // find new centroid location based on average velocity
  vector_t new_loc = body_get_centroid(body);
  new_loc.x = new_loc.x + v_avg.x * dt;
  new_loc.y = new_loc.y + v_avg.y * dt;

  body_set_centroid(body, new_loc);
  body_set_velocity(body, v_new);

  // reset impulse and force
  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  vector_t cur_force = body->force;
  vector_t new_force = vec_add(cur_force, force);
  body->force = new_force;
}

void body_add_impulse(body_t *body, vector_t impulse) {
  vector_t cur_impulse = body->impulse;
  vector_t new_impulse = vec_add(cur_impulse, impulse);
  body->impulse = new_impulse;
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}