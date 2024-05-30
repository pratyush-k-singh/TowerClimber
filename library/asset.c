#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

const size_t FONT_SIZE_1 = 18;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  image_asset_t *asset = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);

  SDL_Texture *texture_init =
      (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  asset->texture = texture_init;
  asset->body = NULL;

  return (asset_t *)asset;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body, double vertical_offset) {
  assert(filepath != NULL);
  assert(body != NULL);

  // Get the bounding box from the body
  SDL_Rect bounding_box;
  get_body_bounding_box(body, &bounding_box, vertical_offset);

  // Initialize the image asset with the bounding box
  image_asset_t *asset = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);

  asset->texture =
      (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  asset->body = body;

  return (asset_t *)asset;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  text_asset_t *asset = (text_asset_t *)asset_init(ASSET_FONT, bounding_box);
  TTF_Font *font_init =
      (TTF_Font *)asset_cache_obj_get_or_create(ASSET_FONT, filepath);
  asset->font = font_init;
  asset->text = text;
  asset->color = color;

  return (asset_t *)asset;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  if (image_asset != NULL) {
    assert(image_asset->type == ASSET_IMAGE);
  }

  if (text_asset != NULL) {
    assert(text_asset->type == ASSET_FONT);
  }

  button_asset_t *button = malloc(sizeof(button_asset_t));
  assert(button);

  button->base = *asset_init(ASSET_BUTTON, bounding_box);
  button->image_asset = (image_asset_t *)image_asset;
  button->text_asset = (text_asset_t *)text_asset;
  button->handler = handler;
  button->is_rendered = false;

  return (asset_t *)button;
}

void asset_on_button_click(asset_t *asset, state_t *state, double x, double y) {
  button_asset_t *button = (button_asset_t *)asset;

  if (!button->is_rendered) {
    return;
  }

  SDL_Rect box = button->base.bounding_box;
  if ((box.x < x && x < box.x + box.w) && (box.y + box.h > y && y > box.y)) {
    button->handler(state);
  }
  button->is_rendered = false;
}

void asset_render(asset_t *asset, double vertical_offset) {
  SDL_Rect box = asset->bounding_box;
  double x = box.x;
  double y = box.y;
  vector_t loc = {x, y};

  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *image = (image_asset_t *)asset;
    SDL_Texture *texture = image->texture;

    if (image->body != NULL) {
      get_body_bounding_box(image->body, &image->base.bounding_box, vertical_offset);
    }

    SDL_Rect box = image->base.bounding_box;
    vector_t loc = {box.x, box.y};
    vector_t size = {box.w, box.h};
    sdl_render_image(texture, loc, size);
    break;
  }

  case ASSET_FONT: {
    text_asset_t *text_asset = (text_asset_t *)asset;
    TTF_Font *font = text_asset->font;
    const char *text = text_asset->text;
    SDL_Color *color = malloc(sizeof(SDL_Color));
    assert(color);
    color->r = text_asset->color.r;
    color->g = text_asset->color.g;
    color->b = text_asset->color.b;
    sdl_render_font(font, text, loc, *color, FONT_SIZE_1);
    break;
  }

  case ASSET_BUTTON: {
    button_asset_t *button = (button_asset_t *)asset;
    asset_render((asset_t *)button->image_asset, vertical_offset);
    if (button->text_asset != NULL) {
      asset_render((asset_t *)button->text_asset, vertical_offset);
    }
    button->is_rendered = true;
    break;
  }
  }
}

void asset_destroy(asset_t *asset) { free(asset); }
