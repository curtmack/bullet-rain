/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * input.c
 * Contains structs and function prototypes for the configurable input system
 */

#include "compile.h"
#include "debug.h"
#include "input.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/* 
 * The input array - if you honestly think you'll need more than 64 inputs,
 * you're probably doing it wrong
 */
input inputs[64];

/* 
 * This function checks if a config type is compatible with an input type
 * e.g. a button works for a boolean input, but not an analog or two-dimensional
 * input
 */
int is_input_compatible(input_type type, input_config_type cfg)
{
    /* Unbinding something is always valid */
    if (cfg == INPUT_NO_BIND) return TRUE;
    
    switch(type) {
        case INPUT_TYPE_BOOLEAN:
            return (cfg == INPUT_KEY || cfg == INPUT_MOUSE_BUTTON ||
                    cfg == INPUT_JOY_BUTTON || cfg == INPUT_JOY_EXTREME ||
                    cfg == INPUT_JOY_HAT);
        case INPUT_TYPE_ANALOG:
            if (cfg == INPUT_JOY_AXIS) return TRUE;
        case INPUT_TYPE_TWODIM:
            /* Two-dimensional inputs work as analog inputs */
            return (cfg == INPUT_MOUSE || cfg == INPUT_JOY_TRACKBALL);
        default:
            /* Nothing is compatible with a non-existant input */
            return FALSE;
    }
}

/* 
 * Update function - this checks an SDL_Event to see if the input system can
 * do anything with it. Returns true if it can and false if it can't.
 */
int update_input(SDL_Event event)
{
    int i, found = FALSE;
    switch (event.type) {
        /* We use the state here, so we don't need to keep these separate */
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            for (i = 0; i < 64; ++i) {
                if (inputs[i].config_type == INPUT_KEY &&
                    inputs[i].key == event.key.keysym.sym) {
                    if (event.key.state == SDL_PRESSED) {
                        inputs[i].valueX = TRUE;
                        inputs[i].valueY = TRUE;
                    }
                    else {
                        inputs[i].valueX = FALSE;
                    }
                    found = TRUE;
                }
            }
            if (found) return TRUE;
            break;
        case SDL_MOUSEMOTION:
            for (i = 0; i < 64; ++i) {
                if (inputs[i].config_type == INPUT_MOUSE) {
                    inputs[i].valueX = event.motion.x;
                    inputs[i].valueY = event.motion.y;
                    found = TRUE;
                }
            }
            if (found) return TRUE;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            for (i = 0; i < 64; ++i) {
                if (inputs[i].config_type == INPUT_MOUSE_BUTTON &&
                    inputs[i].input_num == event.button.button) {
                    if (event.button.state == SDL_PRESSED) {
                        inputs[i].valueX = TRUE;
                        inputs[i].valueY = TRUE;
                    }
                    else {
                        inputs[i].valueX = FALSE;
                    }
                    found = TRUE;
                }
            }
            if (found) return TRUE;
            break;
        case SDL_JOYAXISMOTION:
            for (i = 0; i < 64; ++i) {
                if (inputs[i].config_type == INPUT_JOY_AXIS &&
                    inputs[i].joy_num == event.jaxis.which &&
                    inputs[i].axis_num == event.jaxis.axis) {
                    /* TODO: Implement deadzone stuff */
                    inputs[i].valueX = event.jaxis.value;
                    found = TRUE;
                }
                else if (inputs[i].config_type == INPUT_JOY_EXTREME &&
                         inputs[i].joy_num == event.jaxis.which &&
                         inputs[i].axis_num == event.jaxis.axis) {
                    if (inputs[i].extreme_dir < 0) {
                        inputs[i].valueX =
                            (event.jaxis.value < inputs[i].extreme_threshold);
                    }
                    else {
                        inputs[i].valueX =
                            (event.jaxis.value > inputs[i].extreme_threshold);
                    }
                    found = TRUE;
                }
            }
            if (found) return TRUE;
            break;
        case SDL_JOYBALLMOTION:
            /* TODO: Finish this up, I'm getting bored */
        default:
            /* Input system can't do anything, pass the buck to main */
            return FALSE;
    }
    return FALSE;
}

/* Register functions, these add inputs to the array */
void input_register_boolean(int id)
{
    inputs[id].type = INPUT_TYPE_BOOLEAN;
}
void input_register_analog (int id)
{
    inputs[id].type = INPUT_TYPE_ANALOG;
}
void input_register_twodim (int id)
{
    inputs[id].type = INPUT_TYPE_TWODIM;
}

void unregister_input (int id)
{
    inputs[id].type = INPUT_TYPE_NONE;
}

/* Config functions, these change the event associated with an input */
void input_config_key         (int id, SDLKey key)
{
    if (is_input_compatible(inputs[id].type, INPUT_KEY)) {
        inputs[id].config_type = INPUT_KEY;
        inputs[id].key  = key;
        /* Start off false */
        inputs[id].valueX = FALSE;
        inputs[id].valueY = FALSE;
    }
    else {
        warnn(FALSE, "Invalid bind on input number", id);
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind SDLKey", (int)key);
    }
}

void input_config_mouse       (int id)
{
    if (is_input_compatible(inputs[id].type, INPUT_MOUSE)) {
        inputs[id].config_type = INPUT_MOUSE;
        /* Initialize with the correct position right away */
        SDL_GetMouseState((int*)&(inputs[id].valueX),
                          (int*)&(inputs[id].valueY));
    }
    else {
        warnn(FALSE, "Invalid bind on input number", id);
        verbosen("Input is of type", inputs[id].type);
        verbose("Attempted to bind mouse motion");
    }
}

void input_config_mouse_button(int id, Uint32 button)
{
    if (is_input_compatible(inputs[id].type, INPUT_MOUSE_BUTTON)) {
        inputs[id].config_type = INPUT_MOUSE_BUTTON;
        inputs[id].input_num = button;
        /* Start off false */
        inputs[id].valueX = FALSE;
        inputs[id].valueY = FALSE;
    }
    else {
        warnn(FALSE, "Invalid bind on input number", id);
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind mouse button", button);
    }
}

void input_config_joy_axis    (int id, Uint32 joy, Uint32 axis,
                               Uint16 dz, Uint16 ndz)
{
    if (is_input_compatible(inputs[id].type, INPUT_JOY_AXIS)) {
        inputs[id].config_type = INPUT_JOY_AXIS;
        inputs[id].joy_num = joy;
        inputs[id].axis_num = axis;
        /* Deadzones */
        inputs[id].absolute_deadzone = dz;
        inputs[id].noise_deadzone = ndz;
        /* Start off zeroed */
        inputs[id].valueX = 0;
    }
    else {
        warnn(FALSE, "Invalid bind on input number", id);
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind joy axis", axis);
        verbosen("Of joystick number", joy);
    }
}

void input_config_joy_extreme (int id, Uint32 joy, Uint32 axis,
                               Sint16 dz, Uint16 ndz, Sint16 dir,
                               Uint16 threshold)
{
    if (is_input_compatible(inputs[id].type, INPUT_JOY_AXIS)) {
        inputs[id].config_type = INPUT_JOY_AXIS;
        inputs[id].joy_num = joy;
        inputs[id].axis_num = axis;
        /* Deadzones */
        inputs[id].absolute_deadzone = dz;
        inputs[id].noise_deadzone = ndz;
        /* Threshold information */
        inputs[id].extreme_dir = dir;
        inputs[id].extreme_threshold = threshold;
        /* Start off false */
        inputs[id].valueX = FALSE;
        inputs[id].valueY = FALSE;
    }
    else {
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind joy axis extreme", axis);
        verbosen("Of joystick number", joy);
    }
}

void input_config_joy_button  (int id, Uint32 joy, Uint32 button)
{
    if (is_input_compatible(inputs[id].type, INPUT_JOY_BUTTON)) {
        inputs[id].config_type = INPUT_JOY_BUTTON;
        inputs[id].joy_num = joy;
        inputs[id].input_num = button;
        /* Start off false */
        inputs[id].valueX = FALSE;
        inputs[id].valueY = FALSE;
    }
    else {
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind joy button", button);
        verbosen("Of joystick number", joy);
    }
}
void input_config_joy_hat     (int id, Uint32 joy, Uint32 hat)
{
    if (is_input_compatible(inputs[id].type, INPUT_JOY_HAT)) {
        inputs[id].config_type = INPUT_JOY_HAT;
        inputs[id].joy_num = joy;
        inputs[id].input_num = hat;
        /* Start off false */
        inputs[id].valueX = FALSE;
        inputs[id].valueY = FALSE;
    }
    else {
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind joy POV-hat", hat);
        verbosen("Of joystick number", joy);
    }
}
void input_config_joy_ball    (int id, Uint32 joy, Uint32 ball)
{
    if (is_input_compatible(inputs[id].type, INPUT_JOY_TRACKBALL)) {
        inputs[id].config_type = INPUT_JOY_TRACKBALL;
        inputs[id].joy_num = joy;
        inputs[id].input_num = ball;
        /* Start off zeroed - this is important because trackballs are relative */
        inputs[id].valueX = 0;
        inputs[id].valueY = 0;
    }
    else {
        verbosen("Input is of type", inputs[id].type);
        verbosen("Attempted to bind joy trackball", ball);
        verbosen("Of joystick number", joy);
    }
}

void unbind_input (int id)
{
    inputs[id].config_type = INPUT_NO_BIND;
}

/* Value check functions */
Sint16 input_value  (int id)
{
    return inputs[id].valueX;
}
Sint16 input_pressed(int id)
{
    inputs[id].valueY = FALSE;
    return inputs[id].valueY;
}
void input_twodim_position(int id, Sint16 *x, Sint16 *y)
{
    *x = inputs[id].valueX;
    *y = inputs[id].valueY;
}
void input_ball_position (int id, Sint16 *x, Sint16 *y)
{
    *x = inputs[id].valueX;
    *y = inputs[id].valueY;
}

/* Init/stop functions */
int init_inputs(void)
{
    int i;
    for (i = 0; i < 64; ++i) {
        inputs[i].type = INPUT_TYPE_NONE;
    }
    
    return 0;
}
void stop_inputs(void) {
    /* We don't actually do anything here, it's just a placeholder */
}
