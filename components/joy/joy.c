#include <stdint.h>

#include "esp_adc/adc_oneshot.h"
#include "joy.h"
#include "esp_log.h"

static const char *TAG = "joystick";

static adc_oneshot_unit_handle_t adc1_handle;

static int joy_center_x = 0;
static int joy_center_y = 0;


int32_t joy_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &chan_config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &chan_config));

    ESP_LOGI(TAG, "Calibrating joystick center...");

    int64_t accum_x = 0;
    int64_t accum_y = 0;
    const int SAMPLES_TO_AVG = 64;

    for (int i = 0; i < SAMPLES_TO_AVG; i++) {
        int_fast32_t adc_x;
        int_fast32_t adc_y;

        adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_x);
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &adc_y);

        accum_x += adc_x;
        accum_y += adc_y;
    }

    joy_center_x = (int)(accum_x / SAMPLES_TO_AVG);
    joy_center_y = (int)(accum_y / SAMPLES_TO_AVG);

    ESP_LOGI(TAG, "Joystick center calibrated: X=%d, Y=%d", joy_center_x, joy_center_y);
    return 0;
}


int32_t joy_deinit(void)
{
    if (adc1_handle != NULL) {
        ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
        adc1_handle = NULL;
    }
    return 0;
}


void joy_get_displacement(int32_t *dx, int32_t *dy)
{
    if (adc1_handle == NULL) {
        *dx = 0;
        *dy = 0;
        return;
    }

    int_fast32_t adc_x;
    int_fast32_t adc_y;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_x));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &adc_y));

    *dx = (int)(adc_x - joy_center_x);
    *dy = (int)(adc_y - joy_center_y);
}