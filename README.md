# multibutton

A C library for providing double/triple/quadruple/etc and long clicks for buttons

- [Github repo](https://github.com/codewitch-honey-crisis/htcw_multibutton)
- Arduino library: htcw_multibutton
- Platform IO library: codewitch-honey-crisis/htcw_multibutton
- ESP-IDF component: codewitch-honey-crisis/htcw_multibutton

## Arduino example (allocates)
```cpp
#include <Arduino.h>
#include "multibutton.h"
static multibutton_handle_t mb_handle;
static void mb_on_pressed_changed(bool pressed, void* state) {
    Serial.print("pressed: ");
    Serial.println(pressed?"true":"false");
}
static void mb_on_clicks(unsigned clicks, void* state) {
    Serial.print("clicks: ");
    Serial.println(clicks);
}
static void mb_on_long_click(void* state) {
    Serial.println("long click");
}
void setup(void) {
    Serial.begin(115200);
    pinMode(35,INPUT);
    multibutton_config_t cfg;
    cfg.double_click = 200;
    cfg.long_click = 500;
    cfg.on_pressed_changed_callback = mb_on_pressed_changed;
    cfg.on_clicks_callback = mb_on_clicks;
    cfg.on_long_click_callback = mb_on_long_click;
    cfg.events_size = 0; // use the default
    mb_handle = multibutton_create(&cfg);
}
void loop() {
    static bool old_pressed = false;
    bool pressed = !digitalRead(35);
    if(pressed!=old_pressed) {
        multibutton_event(mb_handle,millis(),pressed);
        old_pressed = pressed;
    }
    multibutton_update(mb_handle,millis());
}
```

## Arduino example (zero allocation)

```cpp
#include <Arduino.h>
#include "multibutton.h"
static multibutton_t mb_data;
static multibutton_handle_t mb_handle;
static multibutton_event_t mb_events[MULTIBUTTON_EVENT_SIZE_DEFAULT];
static void mb_on_pressed_changed(bool pressed, void* state) {
    Serial.print("pressed: ");
    Serial.println(pressed?"true":"false");
}
static void mb_on_clicks(unsigned clicks, void* state) {
    Serial.print("clicks: ");
    Serial.println(clicks);
}
static void mb_on_long_click(void* state) {
    Serial.println("long click");
}
void setup(void) {
    Serial.begin(115200);
    pinMode(35,INPUT);
    multibutton_config_t cfg;
    cfg.double_click = 200;
    cfg.long_click = 500;
    cfg.on_pressed_changed_callback = mb_on_pressed_changed;
    cfg.on_clicks_callback = mb_on_clicks;
    cfg.on_long_click_callback = mb_on_long_click;
    cfg.events_size = MULTIBUTTON_EVENT_SIZE_DEFAULT;
    mb_handle = multibutton_init_za(&cfg,mb_events,&mb_data);
}
void loop() {
    static bool old_pressed = false;
    bool pressed = !digitalRead(35);
    if(pressed!=old_pressed) {
        multibutton_event(mb_handle,millis(),pressed);
        old_pressed = pressed;
    }
    multibutton_update(mb_handle,millis());
}
```

## ESP-IDF example (allocates)
```c
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>
#include "multibutton.h"
static multibutton_handle_t mb_handle;
static void mb_on_pressed_changed(bool pressed, void* state) {
    printf("pressed: %s\n",pressed?"true":"false");
}
static void mb_on_clicks(unsigned clicks, void* state) {
    printf("clicks: %u\n",clicks);
}
static void mb_on_long_click(void* state) {
    puts("long click");
}
void app_main(void) {
    gpio_set_direction(GPIO_NUM_35,GPIO_MODE_INPUT);
    multibutton_config_t cfg;
    cfg.double_click = 200;
    cfg.long_click = 500;
    cfg.on_pressed_changed_callback = mb_on_pressed_changed;
    cfg.on_clicks_callback = mb_on_clicks;
    cfg.on_long_click_callback = mb_on_long_click;
    cfg.events_size = 0; // use the default
    mb_handle = multibutton_create(&cfg);
    TickType_t wdt_ts=xTaskGetTickCount();
    bool old_pressed = false;
    while(1) {
        if(xTaskGetTickCount()>=wdt_ts+pdMS_TO_TICKS(200)) {
            vTaskDelay(5);
        }
        bool pressed = !gpio_get_level(GPIO_NUM_35);
        if(pressed!=old_pressed) {
            multibutton_event(mb_handle,pdTICKS_TO_MS(xTaskGetTickCount()),pressed);
            old_pressed = pressed;
        }
        multibutton_update(mb_handle,pdTICKS_TO_MS(xTaskGetTickCount()));
    }
}
```

## ESP-IDF example (zero-allocation)
```c
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>
#include "multibutton.h"
static multibutton_t mb_data;
static multibutton_handle_t mb_handle;
static multibutton_event_t mb_events[MULTIBUTTON_EVENT_SIZE_DEFAULT];
static void mb_on_pressed_changed(bool pressed, void* state) {
    printf("pressed: %s\n",pressed?"true":"false");
}
static void mb_on_clicks(unsigned clicks, void* state) {
    printf("clicks: %u\n",clicks);
}
static void mb_on_long_click(void* state) {
    puts("long click");
}
void app_main(void) {
    gpio_set_direction(GPIO_NUM_35,GPIO_MODE_INPUT);
    multibutton_config_t cfg;
    cfg.double_click = 200;
    cfg.long_click = 500;
    cfg.on_pressed_changed_callback = mb_on_pressed_changed;
    cfg.on_clicks_callback = mb_on_clicks;
    cfg.on_long_click_callback = mb_on_long_click;
    cfg.events_size = MULTIBUTTON_EVENT_SIZE_DEFAULT;
    mb_handle = multibutton_init_za(&cfg,mb_events,&mb_data);
    TickType_t wdt_ts=xTaskGetTickCount();
    bool old_pressed = false;
    while(1) {
        if(xTaskGetTickCount()>=wdt_ts+pdMS_TO_TICKS(200)) {
            vTaskDelay(5);
        }
        bool pressed = !gpio_get_level(GPIO_NUM_35);
        if(pressed!=old_pressed) {
            multibutton_event(mb_handle,pdTICKS_TO_MS(xTaskGetTickCount()),pressed);
            old_pressed = pressed;
        }
        multibutton_update(mb_handle,pdTICKS_TO_MS(xTaskGetTickCount()));
    }
}
```