#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "watch.h"

static const char *TAG = "lab03";

volatile uint32_t timer_count = 0;
volatile bool running = false;

volatile int64_t isr_max = 0;
volatile int32_t isr_cnt = 0;


bool IRAM_ATTR stopwatch_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    int64_t start = esp_timer_get_time();

    if (pin_get_level(HW_BTN_A) == 0) {
        running = true;
    }

    if (pin_get_level(HW_BTN_B) == 0) {
        running = false;
    }

    if (pin_get_level(HW_BTN_START) == 0) {
        running = false;
        timer_count = 0;
    }

    if (running) {
        timer_count++;
    }

    int64_t finish = esp_timer_get_time();
    int64_t duration = finish - start;
    if (duration > isr_max) {
        isr_max = duration;
    }
    isr_cnt++;

    return false;
}



// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Starting...");

    // Timer variables
    int64_t start, finish;

    // --- Hardware initialization ---
    start = esp_timer_get_time();
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_START);
    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);
    finish = esp_timer_get_time();
    ESP_LOGI(TAG, "Hardware initialized in %lld us", finish - start);


    // --- Timer Setup and Configuration ---
    ESP_LOGI(TAG, "Create timer config...");
    start = esp_timer_get_time();

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = stopwatch_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_LOGI(TAG, "Set up alarm action");
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = 10000, // 10ms at 1MHz
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    ESP_LOGI(TAG, "Start timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    finish = esp_timer_get_time();
    ESP_LOGI(TAG, "Timer configured and started in %lld us", finish - start);


    // --- Main loop ---
    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    ESP_LOGI(TAG, "ESP_LOGI(TAG, \"Stopwatch update\"): %lld us", finish - start);
    
    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_count);

        if (isr_cnt >= 500) {
            ESP_LOGI(TAG, "ISR max time: %lld us", isr_max);
            isr_max = 0;
            isr_cnt = 0;
        }
    }

}