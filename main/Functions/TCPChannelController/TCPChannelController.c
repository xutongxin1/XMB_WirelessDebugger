
#include "sdkconfig.h"
#include <stdint.h>
#include <sys/param.h>


#include "UART/uart_val.h"
#include "TCPChannelController/TCPChannelController.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netbuf.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/priv/api_msg.h"


#define EVENTS_QUEUE_SIZE 10

#ifdef CALLBACK_DEBUG
#define debug(s, ...) printf("%s: " s "\n", "Cb:", ##__VA_ARGS__)
#else
#define debug(s, ...)
#endif
#define MY_IP_ADDR(...) IP4_ADDR(__VA_ARGS__)
static const char *TCP_TAG = "TCP";
static err_t GetConnectState(struct netconn *conn);
//static TaskHandle_t* SubTask_Handle;
//static TaskHandle_t SubTask_Handle_1;
struct netconn *conn_All = NULL;
struct netconn *newconn_All = NULL;
//static uint8_t subtask_one_flag = 0;
//static TaskHandle_t** TcpTaskHandle;
unsigned int kTcpHandleFatherTaskCurrent = 0;
TcpTaskHandleT TCP_TASK_HANDLE[12];
SubTcpParam SubParam = {
    .tcp_param_ = NULL,
    .conn = NULL,
    .newconn_ = NULL,
    .son_task_current_ = 0,
};
// static uint16_t choose_port(uint8_t pin)
// {
//     switch (pin)
//     {
//     case CH2:
//         return 1922;
//     case CH3:
//         return 1923;
//     case CH1:
//         return 1921;
//     default:
//         return -1;
//     }
// }
/**
 * @ingroup xmb tcp server
 * Create a tcp server according to the configuration parameters
 * It is an internal function
 * @param Parameter the configuration parameters
 * @param conn A netconn descriptor
 * @return ERR_OK if bound, any other err_t on failure
 */
uint8_t CreateTcpServer(uint16_t port, struct netconn **conn) {
    while (*conn == NULL) {
        *conn = netconn_new(NETCONN_TCP);
    }
    ESP_LOGI(TCP_TAG,"creat : %p", *conn);
    // netconn_set_nonblocking(conn, NETCONN_FLAG_NON_BLOCKING);
    //netconn_bind(*conn, &ip_info.ip, Param->port);
    /* Bind connection to well known port number 7. */
    netconn_bind(*conn, IP_ADDR_ANY, port);
    netconn_listen(*conn); /* Grab new connection. */
    ESP_LOGI(TCP_TAG,"PORT: %d\nLISTENING.....", port);
    return ESP_OK;
}

//void TcpSendServer(TcpParam *parameter) {
//    // printf("parameter = %p  \n", parameter);
//    TcpParam *param = parameter;
//    // printf("param = %p  \n", param);
//    struct netconn *conn = NULL;
//    // printf("conn\n");
//    struct netconn *newconn = NULL;
//
//    err_t err = 1;
//    // uint16_t len = 0;
//    // void *recv_data;
//    // recv_data = (void *)pvPortMalloc(TCP_BUF_SIZE);
//    // printf("param->uart_queue = %p  \n*param->uart_queue = %p\n", param->uart_queue, *param->uart_queue);
//    QueueHandle_t buff_queue = *param->tx_buff_queue_;
//    //tcpip_adapter_ip_info_t ip_info;
//    /* Create a new connection identifier. */
//    /* Bind connection to well known port number 7. */
//    CreateTcpServer(param->port, &conn);
//    // while (conn == NULL )
//    // {
//    //     conn = netconn_new(NETCONN_TCP);
//    // }
//    // // netconn_set_nonblocking(conn, NETCONN_FLAG_NON_BLOCKING);
//    // //netconn_bind(*conn, &ip_info.ip, param->port);
//    //  /* Bind connection to well known port number 7. */
//    // netconn_bind(conn, IP_ADDR_ANY, param->port);
//    // netconn_listen(conn); /* Grab new connection. */
//    // netconn_set_nonblocking(conn, NETCONN_FLAG_NON_BLOCKING);
//    //MY_IP_ADDR(&ip_info.ip, TCP_IP_ADDRESS);
//    // netconn_bind(conn, &ip_info.ip, param->port);
//
//    /* Tell connection to go into listening mode. */
//    //netconn_listen(conn);
//    //printf("listening : %p\r\n",conn);
//
//    /* Grab new connection. */
//    while (1) {
//        int re_err;
//        err = netconn_accept(conn, &newconn);
//
//        /* Process the new connection. */
//
//        if (err == ERR_OK) {
//
//            struct netbuf *buf;
//            struct netbuf *buf2;
//            void *data;
//            while (1) {
//                re_err = (netconn_recv(newconn, &buf));
//                if (re_err == ERR_OK) {
//                    do {
//                        events event;
//                        netbuf_data(buf, &data, &event.buff_len_);
//                        event.buff_ = data;
//                        if (xQueueSend(buff_queue, &event, pdMS_TO_TICKS(10)) == pdPASS)
//                            ESP_LOGE(TCP_TAG, "TCP_SEND TO QUEUE FAILD\n");
//                    } while ((netbuf_next(buf) >= 0));
//                    netbuf_delete(buf2);
//                    re_err = (netconn_recv(newconn, &buf2));
//                    if (re_err == ERR_OK) {
//                        do {
//                            events event;
//                            netbuf_data(buf2, &data, &event.buff_len_);
//                            event.buff_ = data;
//                            if (xQueueSend(buff_queue, &event, pdMS_TO_TICKS(10)) == pdPASS)
//                                ESP_LOGE(TCP_TAG, "TCP_SEND TO QUEUE FAILD\n");
//                        } while ((netbuf_next(buf2) >= 0));
//                        netbuf_delete(buf);
//                    }
//                } else if (re_err == ERR_CLSD) {
//                    ESP_LOGE(TCP_TAG, "DISCONNECT PORT:%d\n", param->port);
//                    //netbuf_delete(buf);
//                    //netbuf_delete(buf2);
//                    break;
//                }
//            }
//            netconn_close(newconn);
//            netconn_delete(newconn);
//            netconn_listen(conn);
//        }
//    }
//    vTaskDelete(NULL);
//}
//void TcpRevServer(TcpParam *parameter) {
//    TcpParam *param = parameter;
//    struct netconn *conn = NULL;
//    struct netconn *newconn = NULL;
//    // err_t re_err = 0;
//    QueueHandle_t tcp_to_uart_queue_ = *param->UartTcpQueue.tcp_to_uart_queue_;
//
//    /* Create a new connection identifier. */
//    CreateTcpServer(param->port, &conn);
//    // while (conn == NULL)
//    // {
//    //     conn = netconn_new(NETCONN_TCP);
//    //     printf("CONN: %p\n", conn);
//    //     conn->send_timeout = 0;
//    // }
//    // tcpip_adapter_ip_info_t ip_info;
//    // MY_IP_ADDR(&ip_info.ip, TCP_IP_ADDRESS);
//    // // netconn_set_nonblocking(conn, NETCONN_FLAG_NON_BLOCKING);
//    // netconn_bind(conn, &ip_info.ip, param->port);
//    // netconn_bind(conn, IP4_ADDR_ANY, param->port);
//    /* Tell connection to go into listening mode. */
//    //netconn_listen(conn);
//    //printf("PORT: %d\nLISTENING.....\n", param->port);
//    /* Grab new connection. */
//    while (1) {
//        /* Process the new connection. */
//        if (netconn_accept(conn, &newconn) == ERR_OK) {
//            static int ret=0;
//            while (1) {
//                events event;
//                ret = xQueueReceive(tcp_to_uart_queue_, &event, portMAX_DELAY);
//                if (ret == pdTRUE) {
//                    netconn_write(newconn, event.buff_, event.buff_len_, NETCONN_NOCOPY);
//                }
//                if (GetConnectState(newconn) == ERR_CLSD) {
//                    netconn_close(newconn);
//                    netconn_delete(newconn);
//                    netconn_listen(conn);
//                    ESP_LOGE(TCP_TAG, "DISCONNECT PORT:%d\n", param->port);
//                    break;
//                }
//            }
//        }
//    }
//    vTaskDelete(NULL);
//}

void TcpSendServer(SubTcpParam *parameter) {
    ESP_LOGI(TCP_TAG,"ENTERING tcpsendingmode...\n");

    TcpParam *tcp_param = parameter->tcp_param_;
    QueueHandle_t uart_to_tcp_queue_ = *tcp_param->UartTcpQueue.uart_to_tcp_queue_;
    //printf("tcp_rx_queue rx: %p\n", tcp_param->UartTcpQueue.uart_to_tcp_queue_);
    //struct netconn *conn = Param->conn;
    struct netconn *newconn = *(parameter->newconn_);
    TCP_TASK_HANDLE[parameter->son_task_current_].son_task_exists_ = true;
    while (TCP_TASK_HANDLE[parameter->son_task_current_].son_task_exists_) {
        ESP_DRAM_LOGI(TCP_TAG,"tCP SENDING mode is working...\n");

        events event;
        int ret = xQueueReceive(uart_to_tcp_queue_, &event, portMAX_DELAY);
        if (ret == pdTRUE) {
            netconn_write(newconn, event.buff_, event.buff_len_, NETCONN_NOCOPY);
            ESP_LOGI(TCP_TAG,"sending successfully!\n");
        }
        else{
            ESP_LOGE(TCP_TAG,"uart to tcp queue sending error!!\n");
        }
    }
    ESP_LOGE(TCP_TAG,"TCP task is deleted!!!\n");
    vTaskDelete(NULL);
}

TcpTaskHandleT *TcpTaskCreate(TcpParam *parameter,int priority) {
    //printf("parameter uart_to_tcp_queue_:%p\n", parameter->UartTcpQueue.uart_to_tcp_queue_);
    //printf("parameter tcp_to_uart_queue_:%p\n", parameter->UartTcpQueue.tcp_to_uart_queue_);
    //printf("Param:%p\n", parameter);
    const char kAllname[] = "ALL";
//    const char kRxname[] = "Rec";
//    const char kTxname[] = "Tran";
    char pc_name[18];
    printf("\nParam->mode:%d\n", parameter->mode);
    switch (parameter->mode) {
//        case TCP_SEND:sprintf(pc_name, "Tcp%s%d", kTxname, kTcpHandleFatherTaskCurrent);
//            xTaskCreatePinnedToCore((TaskFunction_t) TcpSendServer,
//                                    (const char *const) pc_name,
//                                    5120,
//                                    parameter,
//                                    14,
//                                    TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_handle_,
//                                    0);
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_exists_ = true;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].mode = TCP_SEND;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_port_ = parameter->port;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_taskcount_ = kTcpHandleFatherTaskCurrent;
//            kTcpHandleFatherTaskCurrent++;
//            break;
//        case TCP_RECEIVE:sprintf(pc_name, "Tcp%s%d", kRxname, kTcpHandleFatherTaskCurrent);
//            xTaskCreatePinnedToCore((TaskFunction_t) TcpRevServer,
//                                    (const char *const) pc_name,
//                                    5120,
//                                    parameter,
//                                    14,
//                                    TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_handle_,
//                                    0);
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_exists_ = true;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].mode = TCP_RECEIVE;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_port_ = parameter->port;
//            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_taskcount_ = kTcpHandleFatherTaskCurrent;
//            kTcpHandleFatherTaskCurrent++;
//            break;
        case TCP_ALL:sprintf(pc_name, "Tcp%s%d", kAllname, kTcpHandleFatherTaskCurrent);
            //printf("%s", pc_name);
            xTaskCreatePinnedToCore((TaskFunction_t) TcpServerRevAndSend,
                                    (const char *const) pc_name,
                                    5120,
                                    parameter,
                                    priority,
                                    TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_handle_,
                                    0);
            if (TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_handle_ != NULL) {
                TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_exists_ = true;
                TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].mode = TCP_ALL;
                TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_task_port_ = parameter->port;
            }
            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].father_taskcount_ = kTcpHandleFatherTaskCurrent;
            kTcpHandleFatherTaskCurrent++;
            break;
        default:break;
    }
    return TCP_TASK_HANDLE;
}

uint8_t TcpTaskAllDelete(TcpTaskHandleT *tcp_task_handle_delete) {
    unsigned int need_to_delete_taskcount = 0;
    need_to_delete_taskcount = kTcpHandleFatherTaskCurrent;
    for (int i = 0; i <= need_to_delete_taskcount; i++) {
        if ((tcp_task_handle_delete[i].father_task_exists_) == true) {
            if (tcp_task_handle_delete[i].son_task_handle_ != NULL) {
                tcp_task_handle_delete[i].son_task_exists_ = false;
                vTaskDelete(*(tcp_task_handle_delete[i].son_task_handle_));

            }
            netconn_delete(newconn_All);
            netconn_delete(conn_All);
            vTaskDelete(*(tcp_task_handle_delete[i].father_task_handle_));
            printf("\nDelete%d\n",i);
            tcp_task_handle_delete[i].father_taskcount_ = 0;
            tcp_task_handle_delete[i].mode = 0;
            tcp_task_handle_delete[i].father_task_port_ = 0;
            tcp_task_handle_delete[i].father_task_exists_ = false;
            kTcpHandleFatherTaskCurrent = 0;
        }
    }

    return ESP_OK;
}

///
/// \param parameter
void TcpServerRevAndSend(TcpParam *parameter) {
//    static int time;
    static err_t err = 1;
    static char tmp[16];
    ESP_LOGI(TCP_TAG,"entering receive and send mode");
    ESP_LOGI(TCP_TAG,"MEMP_NUM_NETBUF:%d",MEMP_NUM_NETBUF);
    //printf("tcp_tx_queue tx: %p\n", parameter->UartTcpQueue.uart_to_tcp_queue_);
    /* Create a new connection identifier. */
    CreateTcpServer(parameter->port, &conn_All);//创建TCP Server netCONN协议

    SubParam.tcp_param_ = parameter;
    SubParam.conn = &conn_All;
    SubParam.newconn_ = &newconn_All;
    SubParam.son_task_current_ = kTcpHandleFatherTaskCurrent;

    /* Tell connection to go into listening mode. */
    //netconn_listen(conn);
    // printf("PORT: %d\nLISTENING.....\n",  Param->port);
    ESP_LOGI(TCP_TAG,"begin to accept:%p",&conn_All);
    vTaskDelete(NULL);

    while (1) {
        int re_err;
        ESP_LOGI(TCP_TAG,"begin to accept:%d",parameter->port);
        err = netconn_accept(conn_All, &newconn_All);
        ESP_LOGI(TCP_TAG,"accepted from %d",parameter->port);

        /* Process the new connection. */

        if (err == ERR_OK) {

            if (!TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_task_exists_) {
                /*Create Receive Subtask*/
                SubParam.newconn_ = &newconn_All;
                //sprintf(tmp, "tcp_subtask_%d", parameter->port);
                //printf("\n%s\n", tmp);

                if (xTaskCreatePinnedToCore((TaskFunction_t) TcpSendServer,
                                            (const char *const) tmp,
                                            4096,
                                            (&SubParam),
                                            14,
                                            TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_task_handle_, 0) == pdPASS) {
                    TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_task_exists_ = true;
                    TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_taskcount_++;
                    //strcpy(TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_taskname_, tmp);
                }
                else{
                    ESP_LOGE(TCP_TAG,"tcp sending task is not built!\n");
                }
            }
            /*Create send buffer*/

            void *data;
            while (conn_All->state != NETCONN_CLOSE) {
                struct netbuf *buf;
                re_err = (netconn_recv(newconn_All, &buf));
                if (re_err == ERR_OK) {
                    do{
                        events tx_event_;
                        netbuf_data(buf, &data, &tx_event_.buff_len_);
                        tx_event_.buff_ = data;
//                        if (newconn_All->state != NETCONN_CLOSE) {
//                            netbuf_delete(buf);
//                            break;
//                        }
                        if (xQueueSend(*(parameter->UartTcpQueue.tcp_to_uart_queue_), &tx_event_, pdMS_TO_TICKS(10)) == pdPASS) {

                        }
                        else
                            ESP_LOGE(TCP_TAG, "TCP_SEND TO QUEUE FAILD\n");

                    }while ((netbuf_next(buf) >= 0));
                        netbuf_delete(buf);

                } else if (re_err == ERR_CLSD) {
                    ESP_LOGE(TCP_TAG, "DISCONNECT PORT:%d\n", parameter->port);

                }

            }

//            vTaskDelete(*TCP_TASK_HANDLE[TcpHandle.father_taskcount_].son_task_handle_);
//            is_created_tasks = 0;
//            netconn_close(newconn_);
//            netconn_delete(newconn_);
//            netconn_listen(conn);
        }
    }
    vTaskDelete(NULL);
}

static err_t GetConnectState(struct netconn *conn) {
    void *msg;
    err_t err;
    if (sys_arch_mbox_tryfetch(&conn->recvmbox, &msg) != SYS_MBOX_EMPTY) {
        lwip_netconn_is_err_msg(msg, &err);
        if (err == ERR_CLSD) {
            return ERR_CLSD;
        }
    }
    return ERR_OK;
}