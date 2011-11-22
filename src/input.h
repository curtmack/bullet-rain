/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * input.h
 * Contains structs and function prototypes for the configurable input system
 */

#ifndef INPUT_H

#define INPUT_H

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

typedef enum {
    INPUT_TYPE_NONE,    /* There is no input at this index */
    INPUT_TYPE_BOOLEAN, /* An on/off switch, like a button */
    INPUT_TYPE_ANALOG,  /* An analog axis */
    INPUT_TYPE_TWODIM   /* A two-dimensional input, like a mouse or trackball */
} input_type;

typedef enum {
    INPUT_NO_BIND,      /* This input is unconfigured */
    INPUT_KEY,          /* any key */
    INPUT_MOUSE,        /* mouse motion, both axes */
    INPUT_MOUSE_BUTTON, /* any mouse button */
    INPUT_JOY_AXIS,     /* axis where we need the analog value */
    INPUT_JOY_EXTREME,  /* axis where we only care about a threshold value */
    INPUT_JOY_BUTTON,   /* a button, self explanatory */
    INPUT_JOY_HAT,      /* POV hat - D-pad on most PC joysticks */
    INPUT_JOY_TRACKBALL /* trackball - unlikely, but possible */
} input_config_type;

typedef struct input_ input;
struct input_ {
    input_type        type;
    input_config_type config_type;
    
    /* For keyboard input: the SDLKey event */
    SDLKey key;
    
    /* For mouse motion: X and Y coordinates */
    Uint16 mousex, mousey;
    
    /* Joystick number */
    Uint8 joy_num;
    
    /* For joystick axis/axis extreme inputs: Axis number and deadzones */
    Uint8 axis_num;
    Uint16 absolute_deadzone; /* values less than this are clipped to 0 */
    Uint16 noise_deadzone;    /* changes less than this are ignored */
    
    /* For axis extreme inputs: direction and threshold */
    Sint16 extreme_dir;       /* negative = to min, positive = to max */
    Sint16 extreme_threshold; /* the threshold value */
    
    /* 
     * This is the button number for mouse and joy buttons, the axis number
     * for axis and axis extreme inputs, or the hat/trackball number for those
     * inputs
     */
    Uint8 input_num;
    
    /* Explicit filler */
    Uint8 _pad[3];
    
    /* 
     * This is the axis value for axis inputs, the X-axis for mouse and
     * trackball inputs, a value mask for POV-hats, and a boolean that
     * indicates the pressed state for all other input types.
     */
    Sint16 valueX;
    
    /* 
     * This is the Y-axis for mouse and trackball inputs, and a boolean that
     * indicates the button was pressed since the last time this value was
     * checked for "boolean" inputs. (This is mainly for wheel inputs, which
     * are cleared immediately after being set.)
     */
    Sint16 valueY;
};

/* 
 * This function checks if a config type is compatible with an input type
 * e.g. a button works for a boolean input, but not an analog or two-dimensional
 * input
 */
extern int is_input_compatible(input_type type, input_config_type cfg);

/* 
 * Update function - this checks an SDL_Event to see if the input system can
 * do anything with it. Returns true if it can and false if it can't.
 */
extern int update_input(SDL_Event event);

/* Register functions, these add inputs to the array */
extern void input_register_boolean(int id);
extern void input_register_analog (int id);
extern void input_register_twodim (int id);

extern void unregister_input (int id);

/* Config functions, these change the event associated with an input */
extern void input_config_key         (int id, SDLKey key);
extern void input_config_mouse       (int id);
extern void input_config_mouse_button(int id, Uint32 button);
extern void input_config_joy_axis    (int id, Uint32 joy, Uint32 axis,
                                      Uint16 dz, Uint16 ndz);
extern void input_config_joy_extreme (int id, Uint32 joy, Uint32 axis,
                                      Sint16 dz, Uint16 ndz, Sint16 dir,
                                      Uint16 threshold);
extern void input_config_joy_button  (int id, Uint32 joy, Uint32 button);
extern void input_config_joy_hat     (int id, Uint32 joy, Uint32 hat);
extern void input_config_joy_ball    (int id, Uint32 joy, Uint32 ball);

extern void unbind_input (int id);

/* Value check functions */
extern Sint16 input_value  (int id);
extern Sint16 input_pressed(int id);
extern void input_twodim_position(int id, Sint16 *x, Sint16 *y);

/* Init/stop functions */
extern int  init_inputs(void);
extern void stop_inputs(void);

#endif /* !def INPUT_H */
