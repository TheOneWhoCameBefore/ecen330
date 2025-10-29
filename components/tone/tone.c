#include <stdlib.h>
#include <math.h>
#include "tone.h"
#include "sound.h"
#include "esp_log.h"

static const char *TAG = "TONE";


static uint8_t *waveform_buffer = NULL;
static uint32_t waveform_size = 0;
static uint32_t sample_rate = 0;

#define BIAS 128.0f
#define AMPLITUDE 127.0f


int32_t tone_init(uint32_t sample_hz)
{
    if (sample_hz < (2 * LOWEST_FREQ)) {
        ESP_LOGE(TAG, "Sample rate too low for tone generation");
        return -1;
    }
    sample_rate = sample_hz;

    sound_init(sample_hz);

    waveform_size = sample_rate / LOWEST_FREQ;

    if (waveform_buffer != NULL) {
        free(waveform_buffer);
    }
    waveform_buffer = (uint8_t *)malloc(waveform_size * sizeof(uint8_t));
    if (waveform_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate waveform buffer");
        return -1;
    }

    ESP_LOGI(TAG, "Tone buffer allocated (%u bytes) for %u Hz rate", waveform_size, sample_rate);

    return 0;
}


int32_t tone_deinit(void)
{
    if (waveform_buffer != NULL) {
        free(waveform_buffer);
        waveform_buffer = NULL;
    }

    sound_deinit();
    return 0;
}


void tone_start(tone_t tone, uint32_t freq)
{
    if (waveform_buffer == NULL) {
        ESP_LOGE(TAG, "Tone not initialized");
        return;
    }   
    if (freq < LOWEST_FREQ) {
        freq = LOWEST_FREQ;
    }


    float p = (float)sample_rate / (float)freq;
    uint32_t period_samples = (uint32_t)p;

    if (period_samples > waveform_size) {
        ESP_LOGI(TAG, "Freq %u Hz too low, exceeds buffer size", freq);
        period_samples = waveform_size;
    }

    for (uint32_t i = 0; i < period_samples; i++) {
        float x = (float)i;
        float y = BIAS;

        switch (tone) {
            case SINE_T:
                y = BIAS + AMPLITUDE * sinf((2.0f * M_PI * x) / p);
                break;            
            case SQUARE_T:
                y = (x < p / 2.0f) ? 0xFF : 0x00;
                break;
            case TRIANGLE_T:
                float half_p = p / 2.0f;
                if (x < half_p) {
                    y = (AMPLITUDE / half_p) * x + BIAS;
                } else {
                    y = (-AMPLITUDE / half_p) * x + (AMPLITUDE * 2.0f + BIAS);
                }
                break;
            case SAW_T:
                y = (AMPLITUDE / p) * x + BIAS;
                break;
            case LAST_T:
            default:
                y = BIAS;
                break;
        }

        if (y < 0.0f) y = 0.0f;
        if (y > 255.0f) y = 255.0f;

        waveform_buffer[i] = (uint8_t)y;
    }

    sound_cyclic(waveform_buffer, period_samples);
}