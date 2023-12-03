//
// Created by xtx on 2023/10/26.
//

#ifndef HELLO_WORLD_MAIN_FUNCTIONS_WSLED_WSLED_H_
#define HELLO_WORLD_MAIN_FUNCTIONS_WSLED_WSLED_H_
#include <freertos/freertos.h>
#include "led_strip_rmt.h"
#include <esp_log.h>
#include <freertos/queue.h>
#include "led_strip.h"

extern volatile QueueHandle_t WSQueue_t;
struct WSLED_color
{
    int red;
    int green;
    int blue;
    bool shan;
};
void WSLEDSet(int red,int green,int blue,bool shan);
void WSLEDInit();
void WSLEDRun();
#endif //HELLO_WORLD_MAIN_FUNCTIONS_WSLED_WSLED_H_
