#include "list.h"
#include <assert.h>
#include <stdlib.h>

const size_t GROWTH_FACTOR = 2;

typedef struct list {
  void **data;
  size_t cur_size;
  size_t total_capacity;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_capacity, free_func_t freer) {
  list_t *new_list = malloc(sizeof(list_t));
  assert(new_list);

  new_list->data = malloc(initial_capacity * sizeof(void *));
  assert(new_list->data);

  new_list->total_capacity = initial_capacity;
  new_list->cur_size = 0;

  new_list->freer = freer;
  return new_list;
}

void list_free(list_t *list) {
  if (list == NULL) {
    return; // Nothing to free
  }

  // Free each element in the data array
  if (list->freer != NULL) {
    free_func_t freer = list->freer;
    for (size_t i = 0; i < list->cur_size; i++) {
      freer(list->data[i]);
    }
  }

  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->cur_size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list->cur_size);
  return list->data[index];
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);

  if (list->cur_size >= list->total_capacity) {
    if (list->total_capacity == 0) {
      list->total_capacity = 1;
    }

    // Resize the array if necessary
    size_t new_capacity = list->total_capacity * GROWTH_FACTOR;
    void **new_data = realloc(list->data, new_capacity * sizeof(void *));
    assert(new_data);
    list->data = new_data;
    list->total_capacity = new_capacity;
  }

  list->data[list->cur_size] = value;
  list->cur_size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(index < list->cur_size);
  void *removed = list->data[index];

  // Shift elements to fill the gap
  for (size_t i = index; i < list->cur_size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }

  list->cur_size--;
  return removed;
}
