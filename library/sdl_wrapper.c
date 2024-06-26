#include "sdl_wrapper.h"
#include "asset_cache.h"
#include <SDL2/SDL.h>

#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_mixer.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center, double vertical_offset) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  double scale = get_scene_scale(window_center);
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  scene_center_offset.y -= vertical_offset;
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    exit(1);
  }
  Mix_Volume(-1, MIX_MAX_VOLUME); 
}

bool sdl_is_done(void *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;

    case SDL_MOUSEBUTTONDOWN: {
      asset_cache_handle_buttons(state, event->button.x, event->button.y);
      break;
    }
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

Mix_Chunk *sdl_load_sound(const char *file){
  Mix_Chunk *sound = Mix_LoadWAV(file);
  if (!sound){
    exit(0);
  }
  return sound;
}

void sdl_play_sound(Mix_Chunk *sound){
  Mix_PlayChannel(-1, sound, 0);
}


void sdl_draw_polygon(polygon_t *poly, rgb_color_t color, double vector_offset) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center, vector_offset);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

SDL_Texture *sdl_load_image(const char *image_path) {
  SDL_Texture *image = IMG_LoadTexture(renderer, image_path);
  return image;
}

void sdl_render_image(SDL_Texture *image_texture, vector_t position,
                      vector_t image_size) {
  // Allocate memory for the SDL_Rect structure
  SDL_Rect *img_rect = malloc(sizeof(SDL_Rect));

  // Set the coordinates and dimensions
  img_rect->x = position.x;
  img_rect->y = position.y;
  img_rect->w = image_size.x;
  img_rect->h = image_size.y;

  // Render the image texture
  SDL_RenderCopy(renderer, image_texture, NULL, img_rect);

  // Free the allocated memory
  free(img_rect);
}

void sdl_render_font(TTF_Font *font, const char *text, vector_t position,
                     SDL_Color color, int8_t font_size) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  SDL_Rect destination = {(int)position.x, (int)position.y, surface->w,
                          surface->h};

  SDL_RenderCopy(renderer, texture, NULL, &destination);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

TTF_Font *sdl_load_font(const char *font_path, int8_t font_size) {
  TTF_Font *font = TTF_OpenFont(font_path, font_size);
  return font;
}

void sdl_show(double vector_offset) {
  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, void *aux, double vertical_offset) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  vector_t window_center = get_window_center();

  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);

    for (size_t j = 0; j < list_size(shape); j++) {
      vector_t *point = list_get(shape, j);
      vector_t window_point = get_window_position(*point, window_center, vertical_offset);
      vector_t *new_point = malloc(sizeof(*new_point));
      *new_point = window_point;
    }

    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body), vertical_offset);
    list_free(shape);
  }
  if (aux != NULL) {
  body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body), vertical_offset);
  }
  sdl_show(vertical_offset);
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

void get_body_bounding_box(body_t *body, SDL_Rect *bounding_box, double vertical_offset) {
  list_t *points = body_get_shape(body);
  vector_t window_center = get_window_center();

  double min_x = __DBL_MAX__, max_x = -__DBL_MAX__;
  double min_y = __DBL_MAX__, max_y = -__DBL_MAX__;

  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *point = list_get(points, i);
    vector_t sdl_point = get_window_position(*point, window_center, vertical_offset);

    if (sdl_point.x < min_x)
      min_x = sdl_point.x;
    if (sdl_point.x > max_x)
      max_x = sdl_point.x;
    if (sdl_point.y < min_y)
      min_y = sdl_point.y;
    if (sdl_point.y > max_y)
      max_y = sdl_point.y;
  }

  bounding_box->x = (int)min_x;
  bounding_box->y = (int)min_y;
  bounding_box->w = (int)(max_x - min_x);
  bounding_box->h = (int)(max_y - min_y);
}
