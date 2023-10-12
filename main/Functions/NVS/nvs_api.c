#include "nvs_flash.h"
#include "nvs_api.h"
#include "SwitchMode/SwitchModeHandle.h"

#include <esp_log.h>

static const char *TAG = "NVS";

extern char modeRet[5];
extern int instrucion_kSock;

char need_send_RF = 0;

//写入flash
void NVSFlashWrite(char mode_number) {
    // Initialize NVS
    nvs_flash_erase();
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open
    printf("\n");
    // printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "openDone");

        // Write
        // printf("Updating restart counter in NVS ... ");
        err = nvs_set_i32(my_handle, "mode_number", mode_number);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        // printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    // printf("\n");

    // Restart module
    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
}

//读取flash内容
//读取到的逻辑条件为：上次是为了切换模式而重启，因此在重启前写入flash用于现在读取进入对应模式
int NVSFlashRead() {


    // Initialize NVS
//    esp_err_t err = nvs_flash_init();
//    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//        // NVS partition was truncated and needs to be erased
//        // Retry nvs_flash_init
//        ESP_ERROR_CHECK(nvs_flash_erase());
//        err = nvs_flash_init();
//    }
//    ESP_ERROR_CHECK(err);

    // Open
    // printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err == ESP_OK) {

        // 读取NVS值
        ESP_LOGI(TAG, "Reading restart counter from NVS ... ");
        char mode_number = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "mode_number", (int32_t * ) & mode_number);

        switch (err) {
            case ESP_OK:
                // printf("Done\n");
                ESP_LOGI(TAG, "mode_number = %c", mode_number);

                break;
            case ESP_ERR_NVS_NOT_FOUND:ESP_LOGW(TAG, "The value is not initialized yet!");
                mode_number = 0;
                break;
            default:ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
                return -1;
        }
        nvs_flash_erase();
        // Close
        nvs_close(my_handle);

        if (mode_number > MAX_MODE_NUMBER) {
            ESP_LOGE(TAG, "NVS get error mode number!");
            mode_number = 0;
        }

        if (mode_number != 0) {
            modeRet[2] = (char) (mode_number + '0');
            need_send_RF = 1;
        }
        return mode_number;
    }
    nvs_close(my_handle);
    return -1;
}