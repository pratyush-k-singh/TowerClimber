#include "vector.h"
#include <math.h>

const vector_t VEC_ZERO = {0.0, 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum = {v1.x + v2.x, v1.y + v2.y};
  return sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t diff = {v1.x - v2.x, v1.y - v2.y};
  return diff;
}

vector_t vec_negate(vector_t v) {
  vector_t negate = {-1 * v.x, -1 * v.y};
  return negate;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t multiply = {scalar * v.x, scalar * v.y};
  return multiply;
}

double vec_dot(vector_t v1, vector_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

double vec_cross(vector_t v1, vector_t v2) {
  return (v1.x * v2.y) - (v1.y * v2.x);
}

vector_t vec_rotate(vector_t v, double angle) {
  double rot_x = (v.x * cos(angle)) - (v.y * sin(angle));
  double rot_y = (v.x * sin(angle)) + (v.y * cos(angle));
  vector_t rotate = {rot_x, rot_y};
  return rotate;
}

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }

bool vec_cmp(vector_t v1, vector_t v2){
  return((v1.x == v2.x) && (v1.y == v2.y));
}