#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct entry {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  if (entry == NULL) {
    return;
  }

  if (entry->filepath != NULL) {
    free((void *)entry->filepath);
  }

  switch (entry->type) {
  case ASSET_IMAGE:
    SDL_DestroyTexture((SDL_Texture *)entry->obj);
    break;

  case ASSET_FONT:
    TTF_CloseFont((TTF_Font *)entry->obj);
    break;

  case ASSET_BUTTON:
    asset_destroy(entry->obj);
    break;
  }
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

entry_t *asset_cache_get_entry(const char *filepath) {
  if (filepath != NULL) {
    for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
      entry_t *entry = list_get(ASSET_CACHE, i);
      const char *filecomp = entry->filepath;
      if (filecomp != NULL) {
        if (strcmp(filepath, filecomp) == 0) {
          return entry;
        }
      }
    }
  }
  return NULL;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *entry = asset_cache_get_entry(filepath);

  if (entry != NULL) {
    assert(entry->type == ty);
    return entry->obj;
  }

  void *object = NULL;

  switch (ty) {
  case ASSET_IMAGE:
    object = sdl_load_image(filepath);
    break;
  case ASSET_FONT: {
    object = sdl_load_font(filepath, (int8_t)FONT_SIZE);
    break;
  }
  case ASSET_BUTTON:
    break;
  }
  entry_t *new_entry = malloc(sizeof(entry_t));
  assert(new_entry);

  new_entry->type = ty;
  new_entry->filepath = filepath;
  new_entry->obj = object;
  list_add(ASSET_CACHE, new_entry);

  return object;
}

void asset_cache_register_button(asset_t *button) {
  entry_t *new_button = malloc(sizeof(entry_t));
  assert(new_button);

  assert(asset_get_type(button) == ASSET_BUTTON);
  new_button->type = asset_get_type(button);
  new_button->filepath = NULL;
  new_button->obj = button;
  list_add(ASSET_CACHE, new_button);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *cur_asset = list_get(ASSET_CACHE, i);
    if (cur_asset->type == ASSET_BUTTON) {
      asset_on_button_click((asset_t *)cur_asset->obj, state, x, y);
    }
  }
}
