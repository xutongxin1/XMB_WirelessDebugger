#ifndef __UART_VAL_H__
#define __UART_VAL_H__
#include "driver/uart.h"

//UART_CONGFIG 串口配置参数

#define UART_BUF_SIZE 512
#define TCP_IP_ADDRESS 192, 168, 199, 202

#define EVENT_BUFF_SIZE 512

//串口IO模式
enum UartIOMode {
    UART_IO_MODE_SEND,    //单发
   UART_IO_MODE_RECEIVE, //单收
    UART_IO_MODE_FORWARD, //转发
   UART_IO_MODE_ALL,     //收发
};

//串口引脚配置
#define RX 0
#define TX 1

struct uart_pin
{
    uint8_t rx_pin;
    uint8_t tx_pin;
};

typedef struct 
{
    QueueHandle_t* rx_buff_queue_;
    QueueHandle_t* tx_buff_queue_;
//    QueueHandle_t* forward_buff_queue;
    struct uart_pin pin;
    uart_port_t uart_num;
    enum UartIOMode mode_;
    uart_config_t uart_config_;
}UartInitT;

typedef struct
{
    //enum CommandMode mode;
    char buff_arr_[EVENT_BUFF_SIZE];
    char* buff_;
    uint16_t buff_len_;
}events;

typedef enum 
{
    UART_OK = 0,
    UART_ERROR = -1,
    UART_NUM_EXISTED = -2,
    UART_SET_PAIN_FAIL = -3,
    UART_CONFIG_FAIL = -4,
    UART_INSTALL_FAIL = -5,
    UART_REGISTER_FAIL = -6,
}UartErrT;



typedef struct 
{
    uint8_t rx_pin_;
    uint8_t tx_pin_;
    uint16_t baud_rate_;
    uint8_t data_bit_;
    uint8_t parity_bit_;
    uint8_t stop_bit_;
}UartStateT;

typedef struct
{
    uint8_t task_num_;
    uart_port_t port;
    TaskHandle_t* handle[2];
}UartTaskT;

typedef struct
{
    uart_port_t existed_port_[3];
    UartStateT state[3];
    uint8_t existed_num;
    uint8_t task_num_;
    UartTaskT task_handle_[3];
}UartManageT;



#endif