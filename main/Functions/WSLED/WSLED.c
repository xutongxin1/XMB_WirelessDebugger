//
// Created by xtx on 2023/10/26.
//




#include "WSLED.h"

#define WSLED_GPIO 48
static led_strip_handle_t led_strip;
static const char *TAG = "WSLED";
volatile QueueHandle_t WSQueue_t;
static struct WSLED_color color;

void WSLEDSet(int red,int green,int blue,bool shan) {
    struct WSLED_color color_t;
    color_t.red = red;
    color_t.green = green;
    color_t.blue = blue;
    color_t.shan = shan;
    xQueueSend(WSQueue_t, &color_t, 0);
}
//初始化灯
void WSLEDInit()
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = 48,   // The GPIO that connected to the LED strip's data line
        .max_leds = 1,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags.invert_out = false,                // whether to invert the output signal
    };

    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false, // whether to enable the DMA feature
#endif
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    //初始化值
    color.red = 50;
    color.green = 0;
    color.blue = 0;
    color.shan = true;
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, color.red, color.green, color.blue));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    WSQueue_t = xQueueCreate(5, sizeof(struct WSLED_color));
}

//灯运行任务，该任务意味着wifi一定启动成功
void WSLEDRun() {

    bool now_is_on = true;//是否要求闪烁
    while (1) {
        if (xQueueReceive(WSQueue_t, &color, 0) == pdTRUE) {
            ESP_LOGI(TAG, "red:%d green:%d blue:%d shan:%d", color.red, color.green, color.blue, color.shan);
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, color.red, color.green, color.blue));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        } else if (color.shan) {
            if (now_is_on) {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 0));
            } else {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, color.red, color.green, color.blue));
            }
            now_is_on = !now_is_on;
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
        vTaskDelay(pdMS_TO_TICKS(500));

    }
}