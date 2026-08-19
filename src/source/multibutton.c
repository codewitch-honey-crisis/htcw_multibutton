#include <stdlib.h>
#include <memory.h>
#include "multibutton.h"

static void events_advance(multibutton_handle_t handle) {
    multibutton_t* m = (multibutton_t*)handle;
    if (m->events_full) {
        if (++(m->events_tail) == m->events_size) {
            m->events_tail = 0;
        }
    }

    if (++(m->events_head) == m->events_size) {
        m->events_head = 0;
    }
    m->events_full = (m->events_head == m->events_tail);
}
static void events_retreat(multibutton_handle_t handle) {
    multibutton_t* m = (multibutton_t*)handle;
    m->events_full = false;
    if (++(m->events_tail) == m->events_size) {
        m->events_tail = 0;
    }
}
static inline bool events_empty(multibutton_handle_t handle) {
    multibutton_t* m = (multibutton_t*)handle;
    return (!m->events_full && (m->events_head == m->events_tail));
}
static inline bool events_full(multibutton_handle_t handle) {
    multibutton_t* m = (multibutton_t*)handle;
    return m->events_full;
}
static void events_put(multibutton_handle_t handle,const multibutton_event_t* value) {
    multibutton_t* m = (multibutton_t*)handle;
    m->events[m->events_head]=*value;    
    events_advance(handle);
}
static bool events_get(multibutton_handle_t handle,multibutton_event_t* out_value) {
    multibutton_t* m = (multibutton_t*)handle;
    if (!events_empty(handle)) {
        if (out_value != NULL) {
            *out_value = m->events[m->events_tail];
        }
        events_retreat(handle);
        return true;
    }
    return false;
}

multibutton_handle_t multibutton_init_za(const multibutton_config_t* config, multibutton_event_t* events,  multibutton_t* out_multibutton) {
    
    if(config==NULL || (config->events_size!=0 && config->events_size<2) || events==NULL || out_multibutton==NULL) {
        return NULL;
    }
    out_multibutton->double_click = config->double_click;
    out_multibutton->long_click = config->long_click;
    out_multibutton->events = events;
    out_multibutton->events_size = config->events_size==0?MULTIBUTTON_EVENT_SIZE_DEFAULT:config->events_size;
    out_multibutton->last_state_ts = 0;
    out_multibutton->events_full = false;
    out_multibutton->events_head = 0;
    out_multibutton->events_tail = 0;
    out_multibutton->on_clicks_callback = config->on_clicks_callback;
    out_multibutton->on_clicks_callback_state = config->on_clicks_callback_state;
    out_multibutton->on_long_click_callback = config->on_long_click_callback;
    out_multibutton->on_long_click_callback_state = config->on_long_click_callback_state;
    out_multibutton->on_pressed_changed_callback = config->on_pressed_changed_callback;
    out_multibutton->on_pressed_changed_callback_state = config->on_pressed_changed_callback_state;
    out_multibutton->pressed = false;
    return (multibutton_handle_t)out_multibutton;
}
multibutton_handle_t multibutton_create(const multibutton_config_t* config) {
    if(config==NULL || (config->events_size!=0 && config->events_size<2)) {
        return NULL;
    }
    multibutton_t* result = (multibutton_t*)malloc(sizeof(multibutton_t));
    if(result==NULL) {
        return NULL;
    }
    size_t size = config->events_size==0?MULTIBUTTON_EVENT_SIZE_DEFAULT:config->events_size;
    multibutton_event_t* events = (multibutton_event_t*)malloc(sizeof(multibutton_event_t)*size);
    if(events==NULL) {
        free(result);
        return NULL;
    }
    return multibutton_init_za(config,events,result);
}
void multibutton_destroy(multibutton_handle_t handle) {
    if(handle==NULL) return;
    if(handle->events!=NULL) free(handle->events);
    free(handle);
}
#ifdef IRAM_ATTR
IRAM_ATTR
#endif
bool multibutton_event(multibutton_handle_t handle, uint32_t timestamp, bool pressed) {
    multibutton_t* m = (multibutton_t*)handle;
    bool result = false;
    if (!events_full(handle)) {
        result = true;
        multibutton_event_t e;
        e.ts = timestamp;
        m->last_state_ts = timestamp;
        e.state = pressed;
        events_put(handle,&e);
    }
    m->pressed = pressed;
    if (m->on_pressed_changed_callback != NULL) {
        m->on_pressed_changed_callback(pressed, m->on_pressed_changed_callback_state);
    }
    return result;
}
bool multibutton_pressed(multibutton_handle_t handle) {
    multibutton_t* m = (multibutton_t*)handle;
    return m->pressed;
}
void multibutton_update(multibutton_handle_t handle, uint32_t timestamp) {
    multibutton_t* m = (multibutton_t*)handle;
    if(m->pressed) return;
    if (m->last_state_ts != 0 && !events_empty(handle) && (m->double_click==0 || (timestamp - m->last_state_ts >= m->double_click))) {
        multibutton_event_t ev;
        uint32_t press_ts = 0;
        int state = 0;
        int clicks = 0;
        int longp = 0;
        int done = 0;
        while (!done) {
            switch (state) {
                case 0:
                    if (!events_get(handle,&ev)) {
                        done = true;
                        break;
                    }
                    if (ev.state == 1) {
                        // pressed
                        state = 1;
                        break;
                    } else {
                        // released
                        while (ev.state != 1) {
                            if (!events_get(handle,&ev)) {
                                done = true;
                                break;
                            }
                            // pressed
                            state = 1;
                        }
                        break;
                    }
                case 1:  // press state
                    ++clicks;
                    press_ts = ev.ts;
                    while (ev.state != 0) {
                        if (!events_get(handle,&ev)) {
                            done = true;
                            break;
                        }
                        state = 2;
                    }
                    break;
                case 2:  // release state
                    longp = !!((m->on_long_click_callback && m->long_click) && ev.ts - press_ts >= m->long_click);
                    if (!events_get(handle,&ev)) {
                        // flush the clicks
                        if (m->on_clicks_callback) {
                            if (clicks > longp) {
                                m->on_clicks_callback(clicks - longp, m->on_clicks_callback_state);
                            }
                        }
                        if (longp) {
                            m->on_long_click_callback(m->on_long_click_callback_state);
                        }
                        done = true;
                        break;
                    }
                    state = 1;
                    break;
            }
        }
    }
}