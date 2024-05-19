#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "forces.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "forces.h"
#include "scene.h"

extern size_t INITIAL_CAPACITY;

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
};

typedef struct force_creator_info {
  force_creator_t forcer;
  void *aux;
} force_creator_info_t;

void force_creator_info_free(void *force_info) {
  force_creator_info_t *info = (force_creator_info_t *)force_info;
  body_aux_free(info->aux);
  free(info);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene);
  scene->num_bodies = 0;

  scene->bodies = list_init(INITIAL_CAPACITY, (free_func_t)body_free);
  scene->force_creators = list_init(INITIAL_CAPACITY, force_creator_info_free);

  return scene;
}

void scene_free(scene_t *scene) {
  // Free force creators and their auxiliary data
  list_free(scene->force_creators);
  list_free(scene->bodies);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  body_t *body = list_get(scene->bodies, index);
  assert(body);
  return body;
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_t *bodies = scene->bodies;
  list_add(bodies, body);
  scene->num_bodies++;
}

void scene_remove_body(scene_t *scene, size_t index) {
  // Mark the body for removal
  body_remove(scene_get_body(scene, index));
}

void scene_tick(scene_t *scene, double dt) {
  // Execute all force creators
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    force_creator_info_t *force_info = list_get(scene->force_creators, i);
    force_info->forcer(force_info->aux);
  }

  // Update bodies and remove any that are marked for removal
  ssize_t i = 0;
  while (i < (ssize_t)list_size(scene->bodies)) {
    body_t *body = scene_get_body(scene, i);
    if (body_is_removed(body)) {
      // Remove the associated force creators
      ssize_t j = 0;
      while (j < (ssize_t)list_size(scene->force_creators)) {
        force_creator_info_t *force_info = list_get(scene->force_creators, j);
        for (size_t k = 0; k < list_size(get_bodies_from_aux(force_info->aux));
             k++) {
          if (list_get(get_bodies_from_aux(force_info->aux), k) == body) {
            list_remove(scene->force_creators, j);
            force_creator_info_free(force_info);
            j--;
            break;
          }
        }
        j++;
      }
      // Remove the body
      list_remove(scene->bodies, i);
      body_free(body);
      scene->num_bodies--;
    } else {
      // If the body isn't removed, run body_tick and increment i
      body_tick(body, dt);
      i++;
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, NULL);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  force_creator_info_t *force_info = malloc(sizeof(force_creator_info_t));
  assert(force_info != NULL);

  if (bodies != NULL) {
    list_free(bodies);
  }

  force_info->forcer = forcer;
  force_info->aux = aux;

  list_add(scene->force_creators, force_info);
}
