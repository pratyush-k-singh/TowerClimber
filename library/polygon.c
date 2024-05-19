#include "polygon.h"
#include <math.h>
#include <stdlib.h>

struct polygon {
  list_t *points;
  vector_t velocity;
  double rotation_speed;
  rgb_color_t *color;
  vector_t center;
  double tot_rotation_angle;
};

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  if (points == NULL || list_size(points) == 0) {
    return NULL; // Invalid points list
  }

  polygon_t *polygon = malloc(sizeof(polygon_t));
  if (polygon == NULL) {
    return NULL; // Allocation failed
  }

  polygon->points = points;
  polygon->velocity = initial_velocity;
  polygon->rotation_speed = rotation_speed;
  polygon->color = color_init(red, green, blue); // Create color from RGB
  polygon->center = polygon_centroid(polygon);
  polygon->tot_rotation_angle = 0;
  return polygon;
}

void polygon_free(polygon_t *polygon) {
  if (polygon == NULL) {
    return;
  }
  list_free(polygon->points); // Assumes list_free also frees the elements
  color_free(polygon->color);
  free(polygon);
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  // Update the position of each point based on velocity and time elapsed
  list_t *points = polygon->points;
  size_t num_points = list_size(points);

  // Update the centroid of the polygon
  vector_t center = polygon_get_center(polygon);
  center.x += polygon->velocity.x * time_elapsed;
  center.y += polygon->velocity.y * time_elapsed;
  polygon_set_center(polygon, center);

  // Update every point
  for (size_t i = 0; i < num_points; i++) {
    vector_t *point = list_get(points, i);
    point->x += polygon->velocity.x * time_elapsed;
    point->y += polygon->velocity.y * time_elapsed;
  }
}

void polygon_set_velocity(polygon_t *polygon, vector_t v) {
  polygon->velocity = v;
}

vector_t polygon_get_velocity(polygon_t *polygon) { return polygon->velocity; }

double polygon_area(polygon_t *polygon) {
  list_t *points = polygon_get_points(polygon);
  double area = 0;
  size_t num_points = list_size(points);

  for (size_t i = 0; i < num_points; i++) {
    vector_t *v1 = list_get(points, i);
    vector_t *v2 = list_get(points, (i + 1) % num_points);
    area += vec_cross(*v1, *v2);
  }

  return 0.5 * fabs(area);
}

vector_t polygon_centroid(polygon_t *polygon) {
  double area = polygon_area(polygon); // Calculate area once
  if (area == 0) {
    return VEC_ZERO; // Degenerate polygon
  }
  double x = 0.0, y = 0.0;
  double cross;
  list_t *points = polygon_get_points(polygon);
  size_t num_points = list_size(points);
  for (size_t i = 0; i < num_points; i++) {
    vector_t *v1 = list_get(points, i);
    vector_t *v2 = list_get(points, (i + 1) % num_points);
    cross = vec_cross(*v1, *v2);

    x += ((v1->x + v2->x) * cross);
    y += ((v1->y + v2->y) * cross);
  }
  vector_t centroid = {(1 / (6 * area)) * x, (1 / (6 * area)) * y};
  return centroid;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  list_t *points = polygon->points;
  size_t num_points = list_size(points);

  for (size_t i = 0; i < num_points; i++) {
    vector_t *cur_vector = list_get(points, i);
    cur_vector->x += translation.x;
    cur_vector->y += translation.y;
  }
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  list_t *points = polygon->points;
  size_t num_points = list_size(points);

  for (size_t i = 0; i < num_points; i++) {
    vector_t *cur_vector = list_get(points, i);
    vector_t translated = vec_subtract(*cur_vector, point);
    vector_t rotated = vec_rotate(translated, angle);

    cur_vector->x = rotated.x + point.x;
    cur_vector->y = rotated.y + point.y;
  }
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  polygon->color = color;
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  polygon->center = centroid;
}

vector_t polygon_get_center(polygon_t *polygon) { return polygon->center; }

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon->tot_rotation_angle = rot;
}

double polygon_get_rotation(polygon_t *polygon) {
  return polygon->tot_rotation_angle;
}
