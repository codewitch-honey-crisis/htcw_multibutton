#ifndef HTCW_MULTIBUTTON_H
#define HTCW_MULTIBUTTON_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
#define MULTIBUTTON_EVENT_SIZE_DEFAULT 16

/// @brief The data for a button event (opaque)
typedef struct {
    uint32_t ts;
    int state;
} multibutton_event_t;

/// @brief called when one or more clicks occur in succession
typedef void(*multibutton_on_clicks_callback_t)(unsigned clicks, void* state);
/// @brief called when a long click occurs
typedef void(*multibutton_on_long_click_callback_t)(void* state);
/// @brief called immediately when the pressed state changes
typedef void(*multibutton_on_pressed_changed_callback_t)(bool pressed, void* state);

/// @brief The data for the multibutton (opaque)
typedef struct {
    uint32_t last_state_ts;
    unsigned double_click;
    unsigned long_click;
    size_t events_size;
    size_t events_head;
    size_t events_tail;
    bool events_full;
    multibutton_on_clicks_callback_t on_clicks_callback;
    void* on_clicks_callback_state;
    multibutton_on_long_click_callback_t on_long_click_callback;
    void* on_long_click_callback_state;
    multibutton_on_pressed_changed_callback_t on_pressed_changed_callback;
    void* on_pressed_changed_callback_state;
    multibutton_event_t* events;
    bool pressed;
} multibutton_t;

/// @brief The configuration settings for the multibutton
typedef struct {
    /// @brief Double click time in whatever time units the host provides (typically milliseconds)
    unsigned double_click;
    /// @brief Long click time in whatever time units the host provides (typically milliseconds)
    unsigned long_click;
    /// @brief The number of events to hold. 16 is a good default
    size_t events_size;
    /// @brief The callback to fire when one or more clicks occur in succession
    multibutton_on_clicks_callback_t on_clicks_callback;
    /// @brief The callback state to pass when one or more clicks occur in succession
    void* on_clicks_callback_state;
    /// @brief The callback to fire when a long click occurs
    multibutton_on_long_click_callback_t on_long_click_callback;
    /// @brief The callback state to pass when a long click occurs
    void* on_long_click_callback_state;
    /// @brief The callback to fire when the pressed state changes
    multibutton_on_pressed_changed_callback_t on_pressed_changed_callback;
    /// @brief The callback state to pass when the pressed state changes
    void* on_pressed_changed_callback_state;
} multibutton_config_t;

/// @brief A multibutton handle
typedef multibutton_t* multibutton_handle_t;

/// @brief Initializes a multibutton. Caller allocates
/// @param config The configuration for the button
/// @param events The events buffer. The element count must be the same as the config.events_size
/// @param out_multibutton The structure to hold the multibutton data
/// @return A handle or NULL on error
multibutton_handle_t multibutton_init_za(const multibutton_config_t* config, multibutton_event_t* events,  multibutton_t* out_multibutton);
/// @brief Creates a multibutton.
/// @param config The configuration for the button
/// @return A handle or NULL on error
multibutton_handle_t multibutton_create(const multibutton_config_t* config);
/// @brief Destroys a multibutton created by multibutton_create()
/// @param handle The handle to free
void multibutton_destroy(multibutton_handle_t handle);
/// @brief Raises a button event
/// @param handle The handle to the button
/// @param timestamp The timestamp in whatever time units the host provides (typically milliseconds)
/// @param pressed True if the button is pressed, otherwise false
/// @return True if the event was stored. False if it was dropped due to lack of room
bool multibutton_event(multibutton_handle_t handle, uint32_t timestamp, bool pressed);
/// @brief Indicates whether or not a button is currently pressed
/// @param handle The handle to the button
bool multibutton_pressed(multibutton_handle_t handle);
/// @brief Updates the button by firing any queued events
/// @param handle The handle to the button
/// @param timestamp The current timestamp in whatever time units the host provides (typically milliseconds)
void multibutton_update(multibutton_handle_t handle, uint32_t timestamp);
#ifdef __cplusplus
}
#endif
#endif // HTCW_MULTIBUTTON_H