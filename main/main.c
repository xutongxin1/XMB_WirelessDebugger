/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <nvs_flash.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "Wifi/wifi_handle.h"
#include "NVS/nvs_api.h"
#include "InstructionServer/InstructionServer.h"
#include "SwitchMode/SwitchModeHandle.h"
#include "WSLED/WSLED.h"
void app_main(void) {
    ESP_LOGI("app_main", "APP Run");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //初始化灯
    WSLEDInit();

    //TODO：配网逻辑

    //初始化wifi
    wifi_init();
    WSLEDSet(0,50,0,true);//表示wifi初始化完成
    //灯逻辑
//    xTaskCreatePinnedToCore(WSLEDRun, "WSLEDRun", 4096, NULL, 11, NULL, 1);
    //读取NVS
    int command_mode = NVSFlashRead();//对应泛型
    if (command_mode != -1) {//上次重启是为了配置模式
        working_mode = command_mode;
        ChangeWorkMode(command_mode);//配置工作模式
    }

    // Mode
    // TcpCommandPipeTask 指令通道
    xTaskCreatePinnedToCore(TCPInstructionTask, "TCPInstructionTask", 8192, NULL, 14, NULL, 1);

}
