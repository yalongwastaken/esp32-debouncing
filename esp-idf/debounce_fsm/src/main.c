// Author: Anthony Yalong
// Description: ESP-IDF FSM-based button debouncer with hysteresis for reliable rising-edge detection

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LED_NUM GPIO_NUM_2
#define SWITCH_NUM GPIO_NUM_4

static const char *TAG = "Debounce FSM (ESP-IDF)";

typedef enum {
    DEBOUNCE_LOW,
    DEBOUNCE_RISING,
    DEBOUNCE_HIGH,
    DEBOUNCE_FALLING,
} debounce_state_t;

typedef enum {
    DEBOUNCE_NO_EDGE,
    DEBOUNCE_RISING_EDGE,
    DEBOUNCE_FALLING_EDGE,
} debounce_edge_t;

typedef struct {
    // Setup
    gpio_num_t pin;
    bool inverted;

    // hysteresis
    uint32_t last_update_ticks;
    uint32_t press_delay_ticks;
    uint32_t release_delay_ticks;

    // state
    debounce_state_t state;
} debounce_t;

bool raw_read(gpio_num_t pin, bool inverted) {
    bool raw_result = gpio_get_level(pin);
    return inverted ? !raw_result : raw_result;
}

void gpio_init(void);
void debounce_init(debounce_t* self, gpio_num_t pin, bool inverted, uint32_t press_delay_ms, uint32_t release_delay_ms);
debounce_edge_t debounce_update(debounce_t* self);
void debounce_task(void *pvParameters);

bool led_state;
debounce_t debouncer;
void app_main() {
    gpio_init();
    debounce_init(&debouncer, SWITCH_NUM, true, 50, 20);

    xTaskCreate(debounce_task, "DEBOUNCE BUTTON TASK", 4096, NULL, 1, NULL);
}

void gpio_init(void) {
    // Led setup
    gpio_config_t led_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_NUM),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&led_config);
    led_state = false;
    gpio_set_level(LED_NUM, led_state);

    // Switch setup
    gpio_config_t switch_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << SWITCH_NUM),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&switch_config);
}

void debounce_init(debounce_t* self, gpio_num_t pin, bool inverted, uint32_t press_delay_ms, uint32_t release_delay_ms) {
    // Setup
    self->pin = pin;
    self->inverted = inverted;

    // hysteresis
    self->last_update_ticks = xTaskGetTickCount();
    self->press_delay_ticks = pdMS_TO_TICKS(press_delay_ms);
    self->release_delay_ticks = pdMS_TO_TICKS(release_delay_ms);

    // state init
    self->state = raw_read(pin, inverted) ? DEBOUNCE_HIGH : DEBOUNCE_LOW;
}

debounce_edge_t debounce_update(debounce_t* self) {
    bool raw = raw_read(self->pin, self->inverted);
    TickType_t now = xTaskGetTickCount();

    switch (self->state) {
        case DEBOUNCE_LOW:
            if (raw) {
                self->state = DEBOUNCE_RISING;
                self->last_update_ticks = now;
            }
            break;

        case DEBOUNCE_RISING:
            if (raw) {
                if (now - self->last_update_ticks >= self->press_delay_ticks) {
                    self->state = DEBOUNCE_HIGH;
                    return DEBOUNCE_RISING_EDGE;
                }
            }
            else {
                self->state = DEBOUNCE_LOW;
            }
            break;

        case DEBOUNCE_HIGH:
            if (!raw) {
                self->state = DEBOUNCE_FALLING;
                self->last_update_ticks = now;
            }
            break;

        case DEBOUNCE_FALLING:
            if (!raw) {
                if (now - self->last_update_ticks >= self->release_delay_ticks) {
                    self->state = DEBOUNCE_LOW;
                    return DEBOUNCE_FALLING_EDGE;
                }
            }
            else {
                self->state = DEBOUNCE_HIGH;
            }
            break;
    }

    return DEBOUNCE_NO_EDGE;
}

void debounce_task(void *pvParameters) {
    while (1) {
        debounce_edge_t edge = debounce_update(&debouncer);

        if (edge == DEBOUNCE_RISING_EDGE) {
            led_state = !led_state;
            gpio_set_level(LED_NUM, led_state);
            ESP_LOGI(TAG, "%s", led_state ? "LED ON" : "LED OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}