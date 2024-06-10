#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
    LEFT_ARROW = 1,
    UP_ARROW = 2,
    RIGHT_ARROW = 3,
    DOWN_ARROW = 4,
    SPACE_BAR = 5,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 * @param state a pointer to the current state
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time, void *state);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @param state a pointer to the current state
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Loads a sound from the specified file.
 *
 * @param file the path to the sound file
 * @return a pointer to the loaded sound
 */
Mix_Chunk *sdl_load_sound(const char *file);

/**
 * Plays the specified sound.
 *
 * @param sound a pointer to the sound to play
 */
void sdl_play_sound(Mix_Chunk *sound);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 * @param vector_offset the vertical offset for the polygon position
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color, double vector_offset);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 *
 * @param vector_offset the vertical offset for the scene
 */
void sdl_show(double vector_offset);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 * @param vertical_offset the vertical offset for the scene
 */
void sdl_render_scene(scene_t *scene, void *aux, double vertical_offset);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time, void *state) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Loads a font from the given file path with the specified size.
 *
 * @param font_path the file path to the font file
 * @param font_size the size of the font
 * @return a pointer to the loaded font, or NULL if loading fails
 */
TTF_Font *sdl_load_font(const char *font_path, int8_t font_size);

/**
 * Loads an image from the given file path.
 *
 * @param image_path the file path to the image file
 * @return a pointer to the loaded image texture, or NULL if loading fails
 */
SDL_Texture *sdl_load_image(const char *image_path);

/**
 * Renders text on the SDL window at the specified position.
 *
 * @param font the TTF font to use for rendering
 * @param text the text to render
 * @param position the position at which to render the text
 * @param color the color of the text
 * @param font_size the font size to use for rendering
 */
void sdl_render_font(TTF_Font *font, const char *text, vector_t position, SDL_Color color, int8_t font_size);

/**
 * Renders an image on the SDL window at the specified position with the
 * specified size.
 *
 * @param image_texture the SDL texture representing the image to render
 * @param corner_loc the location at which to render the image
 * @param image_size the size of the image to render
 */
void sdl_render_image(SDL_Texture *image_texture, vector_t corner_loc, vector_t image_size);

/**
 * Calculates the bounding box for a body and stores it in the given SDL_Rect.
 * The bounding box is the smallest rectangle that completely encloses the body.
 *
 * @param body the body for which to calculate the bounding box
 * @param bounding_box a pointer to an SDL_Rect where the bounding box will be stored
 * @param vertical_offset the vertical offset for the bounding box position
 */
void get_body_bounding_box(body_t *body, SDL_Rect *bounding_box, double vertical_offset);

#endif // #ifndef __SDL_WRAPPER_H__
