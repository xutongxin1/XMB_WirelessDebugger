#include "UART/uart_analysis_parameters.h"
#include "cJSON.h"
#include "uart_config.h"
UartInitT c1;
UartInitT c2;
UartInitT c3;

bool c1UartConfigFlag = false; 
bool c2UartConfigFlag = false;
static const char *TAG = "UARTConfiguration";
//串口1接收到的字符解析并进行配置
int Uart1ParameterAnalysis(void *attach_rx_buffer, UartInitT* uart_config) {
    cJSON *pu1 = cJSON_GetObjectItem(attach_rx_buffer, "u1"); // 解析c1字段内容
         //是否指令为空
         if (pu1 != NULL)
         {
            cJSON * item;
             item = cJSON_GetObjectItem(pu1, "mode");
             if (item->valueint == 0) {
                 ESP_LOGI(TAG, "UART1 Not Config");
                 return 0;
             }

             uart_config->pin.rx_pin=5;
             uart_config->pin.tx_pin=4;
             uart_config->mode_ = UART_IO_MODE_ALL;

            //uart_config_->uart_config_.flow_ctrl=UART_HW_FLOWCTRL_DISABLE;

            uart_config->uart_num = UART_NUM_1;
             ESP_LOGI(TAG, "UART1 Config Begin");
             uart_config->mode_ = item->valueint;
            printf("mode = %d\n", uart_config->mode_);



            item=cJSON_GetObjectItem(pu1,"band");
             uart_config->uart_config_.baud_rate = item->valueint;
            printf("band = %d\n", uart_config->uart_config_.baud_rate);

            item=cJSON_GetObjectItem(pu1,"parity");
             uart_config->uart_config_.parity = item->valueint;
            printf("parity = %d\n", uart_config->uart_config_.parity);

            item=cJSON_GetObjectItem(pu1,"data");
             uart_config->uart_config_.data_bits = (item->valueint)-5;
            printf("data = %d\n", uart_config->uart_config_.data_bits);

            item=cJSON_GetObjectItem(pu1,"stop");
             uart_config->uart_config_.stop_bits=item->valueint;
            printf("stop = %d\n", uart_config->uart_config_.stop_bits);
             ESP_ERROR_CHECK(UartSetup(uart_config));
             ESP_LOGI(TAG, "UART1 Config End");
            return 1;
         }
     
    return 0;
}



int Uart2ParameterAnalysis(void *attach_rx_buffer, UartInitT* uart_config) {
    //首先整体判断是否为一个json格式的数据
    cJSON *pu2 = cJSON_GetObjectItem(attach_rx_buffer, "u2"); // 解析c2字段内容

        //是否指令为空
        if (pu2 != NULL)
        {
            cJSON * item;
            item = cJSON_GetObjectItem(pu2, "mode");
            if (item->valueint == 0) {
                ESP_LOGI(TAG, "UART2 Not Config");
                return 0;
            }
            uart_config->pin.rx_pin=7;
            uart_config->pin.tx_pin=6;
            uart_config->uart_num = UART_NUM_2;
            uart_config->mode_ = UART_IO_MODE_ALL;

            ESP_LOGI(TAG, "UART2 Config Begin");
            uart_config->mode_ = item->valueint;
            printf("mode = %d\n", uart_config->mode_);

//            switch (uart_config->mode_) {
//                case Send:
//                case Receive:
//                    return 0;
//                case Forward:
//                case All:
//                    uart_config->mode_ = All;
//                    break;
//                default:
//                    return 0;
  //          }



            item=cJSON_GetObjectItem(pu2,"band");
            uart_config->uart_config_.baud_rate = item->valueint;
            printf("band = %d\n", uart_config->uart_config_.baud_rate);
            
            item=cJSON_GetObjectItem(pu2,"parity");
            uart_config->uart_config_.parity = item->valueint;
            printf("parity = %d\n", uart_config->uart_config_.parity);
            
            item=cJSON_GetObjectItem(pu2,"data");
            uart_config->uart_config_.data_bits = (item->valueint)-5;
            printf("data = %d\n", uart_config->uart_config_.data_bits);
            
            item=cJSON_GetObjectItem(pu2,"stop");
            uart_config->uart_config_.stop_bits = item->valueint;
            printf("stop = %d\n", uart_config->uart_config_.stop_bits);
            ESP_ERROR_CHECK(UartSetup(uart_config));
            ESP_LOGI(TAG, "UART2 Config End");
            
            return 1;
        }
    ESP_LOGI(TAG, "UART2 Not Config");
    return 0;
}

