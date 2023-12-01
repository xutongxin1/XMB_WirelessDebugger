#include "SwitchModeHandle.h"


#include "UART/uart_analysis_parameters.h"
#include "UART/uart_config.h"

#include "lwip/sockets.h"
#include "TCPChannelController/TCPChannelController.h"
#include <esp_log.h>

//TcpParam tcp_param;
enum WorkMode working_mode = NONE_MODE;
extern UartInitT c1;
extern UartInitT c2;
extern bool c1UartConfigFlag;
extern bool c2UartConfigFlag;
extern const char kHeartRet[5]; // 心跳包发送
extern uint8_t uart1_flag;
extern uint8_t uart2_flag;
TcpInfoTaskHandleT *tcphand1;
TcpInfoTaskHandleT *tcphand2;
unsigned portBASE_TYPE uxHighWaterMark_tcp1;
unsigned portBASE_TYPE uxHighWaterMark_tcp2;
extern const int kTcpHandleFatherTaskCurrent;
extern TcpInfoTaskHandleT tcp_info_task_handle[4];
//TcpTaskHandleT *tcphand1;
//TcpTaskHandleT *tcphand2;
//QueueHandle_t uart_queue1 = NULL;
//QueueHandle_t uart_queue = NULL;
//QueueHandle_t uart_queue2 = NULL;
//QueueHandle_t uart_queue3 = NULL;
static const char *TAG = "ModeSwitcher";

void UartHandle(void) {
    ESP_LOGI(TAG, "Enter Uart Mode");
    working_mode = UART;
}

void ADCHandle(void) {}

void DACHandle(void) {}

void PwmCollectHandle(void) {}

void PwmSimulationHandle(void) {}

void I2CHandle(void) {}

void SpiHandle(void) {}

void CanHandle(void) {}

void UartTask(int ksock) {
    int written = 0;

    static QueueHandle_t tcp1_to_uart1_queue_ = NULL;
    static QueueHandle_t uart1_to_tcp1_queue_ = NULL;
    static QueueHandle_t uart2_to_tcp2_queue_ = NULL;
    static QueueHandle_t tcp2_to_uart2_queue_ = NULL;
    uart1_to_tcp1_queue_ = xQueueCreate(50, sizeof(events));
    tcp1_to_uart1_queue_ = xQueueCreate(50, sizeof(events));
    uart2_to_tcp2_queue_ = xQueueCreate(50, sizeof(events));
    tcp2_to_uart2_queue_ = xQueueCreate(50, sizeof(events));

    c1.UartTcpQueue.uart_to_tcp_queue_ = &uart1_to_tcp1_queue_;
    c1.UartTcpQueue.tcp_to_uart_queue_ = &tcp1_to_uart1_queue_;
    c2.UartTcpQueue.tcp_to_uart_queue_ = &tcp2_to_uart2_queue_;
    c2.UartTcpQueue.uart_to_tcp_queue_ = &uart2_to_tcp2_queue_;
    printf("c2 uart_to_tcp_queue_:%p\n", (c2.UartTcpQueue.uart_to_tcp_queue_));
    printf("c2 tcp_to_uart_queue_:%p\n", (c2.UartTcpQueue.tcp_to_uart_queue_));

    printf("uart1_to_tcp1_queue_ rx: %p  ", &uart1_to_tcp1_queue_);
    printf("uart1_tx_tcp_queuetx: %p", &tcp1_to_uart1_queue_);

    // xTaskCreatePinnedToCore(UartRev, "uartr", 5120, (void *)&c1, 10, &xHandle, 0);
    if (c1UartConfigFlag == true) {

        if (uart1_flag == 0) {//串口1已被占用，重新配置
            CreateUartTask(&c1);
        }
        ESP_ERROR_CHECK(TcpTaskAllDelete(&tcp_info_task_handle[TCPINFOCH1]));

        tcp_info_task_handle[TCPINFOCH1].mode_ = TCP_ALL;
        tcp_info_task_handle[TCPINFOCH1].port_ = Port1921;
        tcp_info_task_handle[TCPINFOCH1].uart_tcp_queue_.tcp_to_uart_queue_ = &tcp1_to_uart1_queue_;
        tcp_info_task_handle[TCPINFOCH1].uart_tcp_queue_.uart_to_tcp_queue_ = &uart1_to_tcp1_queue_;
        ESP_ERROR_CHECK(TcpTaskCreate(&tcp_info_task_handle[TCPINFOCH1], 13));

        c1UartConfigFlag = false;
    }

    if (c2UartConfigFlag == true) {
        if (uart2_flag == 0) {//串口1已被占用，重新配置
            CreateUartTask(&c2);
        }
        ESP_ERROR_CHECK(TcpTaskAllDelete(&tcp_info_task_handle[TCPINFOCH2]));

        tcp_info_task_handle[TCPINFOCH2].mode_ = TCP_ALL;
        tcp_info_task_handle[TCPINFOCH2].port_ = Port1922;
        tcp_info_task_handle[TCPINFOCH2].uart_tcp_queue_.tcp_to_uart_queue_ = &tcp2_to_uart2_queue_;
        tcp_info_task_handle[TCPINFOCH2].uart_tcp_queue_.uart_to_tcp_queue_ = &uart2_to_tcp2_queue_;
        ESP_ERROR_CHECK(TcpTaskCreate(&tcp_info_task_handle[TCPINFOCH2], 13));

        c2UartConfigFlag = false;
    }

    do {
        written = send(ksock, "OK!\r\n", 5, 0);
    } while (written <= 0);
}