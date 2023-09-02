#include "sdkconfig.h"


#include <stdint.h>
#include <sys/param.h>


#include "UART/uart_val.h"
#include "UART/uart_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/uart.h"


const char *UART_TAG = "UART";

static TaskHandle_t uart_task_handle[4];

static UartManageT uart_manage;
uint8_t uart_flag = 0;
// void UartRev(void *uartParameter)
// {
//     uart_configrantion config = *(uart_configrantion *)uartParameter;
//     uart_port_t uart_num = config.uart_num;
//     UartSetup(&config);
//     char buffer[UART_BUF_SIZE];
//     int uart_buf_len = 0;
//     events event;
//     QueueHandle_t uart_queue = *config.buff_queue;
//     while (1)
//     {
//         uart_get_buffered_data_len(uart_num, (size_t *)&uart_buf_len);
//         if (uart_buf_len)
//         {
//             uart_buf_len = uart_buf_len > UART_BUF_SIZE ? UART_BUF_SIZE : uart_buf_len;
//             uart_buf_len = uart_read_bytes(uart_num, buffer, uart_buf_len, pdMS_TO_TICKS(5));
//             buffer[uart_buf_len] = '\0';
//             ESP_LOGE(UART_TAG, "buffer = %s  \nuart_buf_len = %d\n", buffer, uart_buf_len);
//             if (uart_buf_len != 0)
//             {
//                 strncpy(event.buff_, buffer, uart_buf_len);
//                 ESP_LOGE(UART_TAG, "event buffer = %s  \n", event.buff_);
//                 event.buff_len_ = uart_buf_len;
//                 uart_buf_len = 0;
//                 if (xQueueSend(uart_queue, &event, pdMS_TO_TICKS(10)) == pdTRUE)
//                 {
//                     ESP_LOGE(UART_TAG, "TCP_SEND TO QUEUE\n");
//                 }
//                 else
//                 {
//                     ESP_LOGE(UART_TAG, "TCP_SEND TO QUEUE FAILD\n");
//                 }
//             }
//         }
//     }
//     vTaskDelete(NULL);
// }

// void UartSend(void *uartParameter)
// {
//     uart_configrantion config = *(uart_configrantion *)uartParameter;
//     uart_port_t uart_num = config.uart_num;
//     UartSetup(&config);
//     // ESP_LOGE(UART_TAG, "config.uart_queue = %p  \n*config.uart_queue = %p\n", config.buff_queue, *config.buff_queue);
//     QueueHandle_t uart_queue = *config.buff_queue;
//     while (1)
//     {
//         events event;
//         // ESP_LOGE(UART_TAG, "&event: %p", &event);
//         while (xQueueReceive(uart_queue, &event, pdMS_TO_TICKS(100)) != pdTRUE)
//             ;
//         if (event.buff_len_ != 0)
//         {
//             uart_write_bytes(uart_num, (const char *)event.buff_, event.buff_len_);
//         }
//     }
//     vTaskDelete(NULL);
// }

// void UartSetup(UartInitT *config)
// {
//     ESP_LOGE(UART_TAG, "pin.ch = %d  pin.mode = %d,\n", config->pin.CH, config->pin.MODE);
//     if (config->pin.MODE == TX)
//     {
//         uart_set_pin(config->uart_num, config->pin.CH, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//         ESP_LOGE(UART_TAG, "config->uart_num = %d\n", config->uart_num);
//         uart_param_config(config->uart_num, &config->uart_config_);
//         ESP_ERROR_CHECK(uart_driver_install(config->uart_num, 129, UART_BUF_SIZE, 0, NULL, 0));
//         ESP_LOGE(UART_TAG, "uart bridge init successfully\n");
//     }
//     else
//     {
//         uart_set_pin(config->uart_num, UART_PIN_NO_CHANGE, config->pin.CH, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//         ESP_LOGE(UART_TAG, "config->uart_num = %d\n", config->uart_num);
//         uart_param_config(config->uart_num, &config->uart_config_);
//         ESP_ERROR_CHECK(uart_driver_install(config->uart_num, UART_BUF_SIZE, 0, 0, NULL, 0));
//         ESP_LOGE(UART_TAG, "uart bridge init successfully\n");
//     }
// }
//询问输入的串口号是否空闲，空闲则返回串口号
uint8_t GetUartManageId(uart_port_t uart_num) {
    if (uart_num == UART_NUM_0) {
        return -1;
    }
    for (int i = 0; i < uart_manage.existed_num; i++) {
        if (uart_manage.existed_port_[i] == uart_num) {
            return i;
        }
    }
    return 0;
}
UartErrT UartStateRegister(UartInitT *config) {
    UartInitT *param = config;
    uart_port_t uart_num = param->uart_num;
    uint8_t id = uart_manage.existed_num;
    uart_manage.existed_num++;//记录串口使用数量
    if (id == 2) {//两个串口都用了
        return UART_ERROR;
    }
    uart_manage.existed_port_[id] = uart_num;
    uart_manage.task_handle_[id].port = uart_num;
    uart_manage.state[id].rx_pin_ = param->pin.rx_pin;
    uart_manage.state[id].tx_pin_ = param->pin.tx_pin;
    uart_manage.state[id].baud_rate_ = param->uart_config_.baud_rate;
    uart_manage.state[id].data_bit_ = param->uart_config_.data_bits;
    uart_manage.state[id].parity_bit_ = param->uart_config_.parity;
    uart_manage.state[id].stop_bit_ = param->uart_config_.stop_bits;
    return UART_OK;
}


uart_port_t GetUartFreeNum() {//返回当前可用的串口号
    if (IsUartNumFree(UART_NUM_1) == UART_OK) {
        return UART_NUM_1;
    }
    if (IsUartNumFree(UART_NUM_2) == UART_OK) {
        return UART_NUM_2;
    }
    return UART_NUM_0;
}

UartErrT IsUartNumFree(uart_port_t uart_num) {
    if (uart_num == UART_NUM_0) {
        return UART_NUM_EXISTED;
    }
    for (int i = 0; i < uart_manage.existed_num; i++) {
        if (uart_manage.existed_port_[i] == uart_num) {
            return UART_NUM_EXISTED;

        }
    }
    return UART_OK;
}




UartErrT UartSetup(UartInitT *config)
{
    uart_port_t tmp_num = config->uart_num;
    config->uart_config_.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config->uart_config_.source_clk = UART_SCLK_APB;
    config->uart_num = GetUartFreeNum();

    // config->uart_num = config->uart_num;
    if (IsUartNumFree(UART_NUM_1))//如果串口1已经被使用，需要切换模式，标志位 置1，
    {
        // ESP_LOGE(UART_TAG, "uart NUM existed\r\n");
        // return UART_NUM_EXISTED;
        uart_flag = 1;
    }

    if (uart_flag == 0)//串口空闲，直接配置
    {
        if (UartStateRegister(config))
        {
            ESP_LOGE(UART_TAG, "uart register fail\r\n");
            return UART_REGISTER_FAIL;
        }
        if (uart_set_pin(tmp_num, config->pin.tx_pin, config->pin.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE))
        {

            ESP_LOGE(UART_TAG, "uart set pin fail\r\n");
            return UART_SET_PAIN_FAIL;
        }
        if (uart_param_config(config->uart_num, &config->uart_config_)) {
            ESP_LOGE(UART_TAG, "uart config fail\n");
            return UART_CONFIG_FAIL;
        }
        if (uart_driver_install(config->uart_num, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0)) {
            ESP_LOGE(UART_TAG, "uart init fail\n");
            return UART_INSTALL_FAIL;

        }
    }
    else if (uart_flag == 1)//串口正在使用中，需要删除驱动，重新配置为目标模式
    {
        printf("uart_flag:%d\n", uart_flag);
        config->uart_num = tmp_num;
        UartStateRegister(config);
        uart_driver_delete(config->uart_num);         
        uart_set_baudrate(config->uart_num, config->uart_config_.baud_rate);
        uart_set_word_length(config->uart_num, config->uart_config_.data_bits);
        uart_set_parity(config->uart_num, config->uart_config_.parity);
        uart_set_stop_bits(config->uart_num, config->uart_config_.stop_bits);
        uart_set_hw_flow_ctrl(config->uart_num, UART_HW_FLOWCTRL_DISABLE, 0);
        uart_set_pin(config->uart_num, config->pin.tx_pin, config->pin.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_driver_install(config->uart_num, UART_BUF_SIZE , UART_BUF_SIZE ,0,NULL,0);
    }
    return UART_OK;
}

UartErrT CreateUartTask(UartInitT *uart_config) {
    UartErrT err;
    err = UartSetup(uart_config);
    if (err) {
        return err;
    }
    uint8_t id = GetUartManageId(uart_config->uart_num);
    const char kAllname[] = "ALL";
    const char kRxname[] = "Rec";
    const char kTxname[] = "Tran";
    char pc_name[18];
    switch (uart_config->mode_)
    {
    case UART_IO_MODE_SEND:
        sprintf(pc_name, "Uart%s%d", kTxname, uart_manage.task_num_);
            xTaskCreatePinnedToCore((TaskFunction_t) UartSend,
                                    (const char *const) pc_name,
                                    5120,
                                    uart_config,
                                    14,
                                    &uart_task_handle[uart_manage.task_num_],
                                    1);
        uart_manage.task_num_++;
        uart_manage.task_handle_[id].handle[uart_manage.task_handle_[id].task_num_] = &uart_task_handle[uart_manage.task_num_];
        uart_manage.task_handle_[id].task_num_++;
        break;
    case UART_IO_MODE_RECEIVE:
        sprintf(pc_name, "Uart%s%d", kRxname, uart_manage.task_num_);
            xTaskCreatePinnedToCore((TaskFunction_t) UartRev,
                                    (const char *const) pc_name,
                                    5120,
                                    uart_config,
                                    14,
                                    &uart_task_handle[uart_manage.task_num_],
                                    1);
        uart_manage.task_num_++;
        uart_manage.task_handle_[id].handle[uart_manage.task_handle_[id].task_num_] = &uart_task_handle[uart_manage.task_handle_[id].task_num_];
        uart_manage.task_handle_[id].task_num_++;
        break;
    case UART_IO_MODE_ALL:
        sprintf(pc_name, "Uart%s%s%d", kAllname, kTxname, uart_manage.task_num_);
            xTaskCreatePinnedToCore((TaskFunction_t) UartSend,
                                    (const char *const) pc_name,
                                    5120,
                                    uart_config,
                                    14,
                                    &uart_task_handle[uart_manage.task_num_],
                                    1);
        uart_manage.task_num_++;
        uart_manage.task_handle_[id].handle[uart_manage.task_handle_[id].task_num_] = &uart_task_handle[uart_manage.task_num_];
        uart_manage.task_handle_[id].task_num_++;
        sprintf(pc_name, "Uart%s%s%d", kAllname, kRxname, uart_manage.task_num_);
            xTaskCreate((TaskFunction_t) UartRev,
                        (const char *const) pc_name,
                        5120,
                        uart_config,
                        14,
                        &uart_task_handle[uart_manage.task_num_]);
        uart_manage.task_num_++;
        uart_manage.task_handle_[id].handle[uart_manage.task_handle_[id].task_num_] = &uart_task_handle[uart_manage.task_num_];
        uart_manage.task_handle_[id].task_num_++;
        break;
        case UART_IO_MODE_FORWARD:
            //todo:收发功能

    default:
        break;
    }
    return UART_OK;
}

UartErrT Delete_All_Uart_Task() {
    int i_max = uart_manage.existed_num;
    int j_max = 0;
    for (int i = 0; i < i_max; i++) {
        j_max = uart_manage.task_handle_[i].task_num_;
        for (int j = 0; j < j_max; j++) {
            if (uart_manage.task_handle_[i].handle[j] != NULL) {
                vTaskDelete(*uart_manage.task_handle_[i].handle[j]);
                uart_manage.task_num_--;
            }
        }
        uart_manage.task_handle_[i].task_num_ = 0;
    }
    return UART_OK;
}

void UartSend(UartInitT *uart_config) {
    uart_port_t uart_num = uart_config->uart_num;
    while (1) {
        events event;
        // ESP_LOGE(UART_TAG, "&event: %p", &event);
        while (xQueueReceive(*(uart_config->tx_buff_queue_), &event, pdMS_TO_TICKS(100)) != pdTRUE);
        if (event.buff_len_ != 0) {
            uart_write_bytes(uart_num, (const char *) event.buff_, event.buff_len_);
        }
    }
    vTaskDelete(NULL);
}

void UartRev(UartInitT *uart_config) {
    uart_port_t uart_num = uart_config->uart_num;
//    char buffer[UART_BUF_SIZE];
    int uart_buf_len = 0;
    events event;
    while (1) {
        uart_get_buffered_data_len(uart_num, (size_t *) &uart_buf_len);
        if (uart_buf_len) {
            uart_buf_len = uart_buf_len > UART_BUF_SIZE ? UART_BUF_SIZE : uart_buf_len;
            uart_buf_len = uart_read_bytes(uart_num, event.buff_arr_, uart_buf_len, pdMS_TO_TICKS(5));
            event.buff_arr_[uart_buf_len] = '\0';
            // ESP_LOGE(UART_TAG, "buffer = %s  \nuart_buf_len = %d\n", buffer, uart_buf_len);
            if (uart_buf_len != 0) {
                // strncpy(event.buff_, buffer, uart_buf_len);
                // ESP_LOGE(UART_TAG, "event buffer = %s  \n", event.buff_arr_);
                event.buff_ = event.buff_arr_;
                event.buff_len_ = uart_buf_len;
                uart_buf_len = 0;
                if (xQueueSend(*(uart_config->rx_buff_queue_), &event, pdMS_TO_TICKS(10)) == pdTRUE) {
                    //ESP_LOGI(UART_TAG, "TCP_SEND TO QUEUE\n");
                } else {
                    ESP_LOGE(UART_TAG, "TCP_SEND TO QUEUE FAILD\n");
                }
            }
        } else {
            vTaskDelay(5);
        }
    }
    vTaskDelete(NULL);
}
