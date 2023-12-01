#include <sys/cdefs.h>
#ifndef _TCP_H_
#define _TCP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
//TCP相关
//#define TCP_BUF_SIZE 512
//#define QUEUE_BUF_SIZE 64

enum TCPPort
{
    InstructionPort = 1920,
    Port1921,
    Port1922,
    Port1923
};
enum TCPInfoChannle {
    TCPINFOCH1 = 0,
    TCPINFOCH2,
    TCPINFOCH3,
};

enum TCPMode {
    TCP_ALL = 1,
//    TCP_SEND ,
//    TCP_RECEIVE,
};


typedef struct
{
    enum TCPMode mode_;
    enum TCPPort port_;

    TaskHandle_t *k_father_task_;//TCP接受连接与接收的句柄
    TaskHandle_t *k_son_task_;//TCP发送的句柄

    bool father_task_exists_ ;
    bool son_task_exists_;

    //功能性队列参数
    UartTcpQueueT uart_tcp_queue_;

    //NetIF句柄
    struct netconn *k_server_;
    struct netconn *k_client_;

} TcpInfoTaskHandleT;
extern TcpInfoTaskHandleT tcp_info_task_handle[4];//1921,1922,1923,1924
//void TcpSendServer(TcpParam *parameter);
//void TcpRevServer(TcpParam *parameter);
/// TCP接受连接与接收的任务
/// \param parameter 传参
_Noreturn void TcpServerAcceptWithRec(TcpInfoTaskHandleT *parameter);

/// TCP发送的任务
/// \param parameter 传参
_Noreturn void TcpServerSend(TcpInfoTaskHandleT *parameter);

/// 创建TCP NetIF Server句柄并监听
/// \param port 端口
/// \param k_server NetIF服务端句柄
/// \return 返回创建是否成功
esp_err_t CreateTcpServer(uint16_t port, struct netconn *k_server);

/// 删除TCP NetIF 所有句柄和任务
/// \param tcp_task_handle_delete
/// \return 删除是否成功
esp_err_t TcpTaskAllDelete(TcpInfoTaskHandleT *tcp_task_handle_delete);

/// TCP任务创建
/// \param parameter 传参
/// \param priority 任务优先级
/// \return 创建是否成功
esp_err_t TcpTaskCreate(TcpInfoTaskHandleT *parameter, int priority);

#endif