#ifndef _TCP_H_
#define _TCP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
//TCP相关
#define TCP_BUF_SIZE 512
#define QUEUE_BUF_SIZE 64
// enum CHIo
// {
//     CH1 = 34,
//     CH2 = 25,
//     CH3 = 26,

// };

enum Port
{
    CH1 = 1920,
    CH2,
    CH3,
    CH4


};

enum TCPMode {
    TCP_SEND = 1,
    TCP_RECEIVE,
    TCP_ALL,
};

typedef struct 
{
    QueueHandle_t* tx_buff_queue_;
    QueueHandle_t* rx_buff_queue_;
    enum TCPMode mode;
    enum Port port;
}TcpParam;

typedef struct 
{
    TcpParam* tcp_param_;
    struct netconn **conn;
    struct netconn **newconn_;
    uint8_t son_task_current_;
}SubTcpParam;

typedef struct 
{
    TaskHandle_t* father_task_handle_;
    uint8_t father_taskcount_;
    enum Port father_task_port_;
    TaskHandle_t* son_task_handle_;
    uint8_t son_taskcount_;
    char son_taskname_[16];
    bool father_task_exists_;
    bool son_task_exists_;
    enum TCPMode mode;
} TcpTaskHandleT;

void TcpSendServer(TcpParam *parameter);
void TcpRevServer(TcpParam *parameter);
void TcpServerRevAndSend(TcpParam *parameter);
uint8_t CreateTcpServer(uint16_t port, struct netconn **conn);
uint8_t TcpTaskAllDelete(TcpTaskHandleT* tcp_task_handle_delete);
TcpTaskHandleT* TcpTaskCreate(TcpParam *parameter);
void TcpServerSubtask(SubTcpParam *parameter);
#endif