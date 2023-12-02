#include <sys/cdefs.h>
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


#ifdef CALLBACK_DEBUG
#define debug(s, ...) printf("%s: " s "\n", "Cb:", ##__VA_ARGS__)
#else
#define debug(s, ...)
#endif
#define MY_IP_ADDR(...) IP4_ADDR(__VA_ARGS__)
static const char *TCP_TAG = "TCP";

static err_t GetConnectState(struct netconn *conn);

TcpInfoTaskHandleT tcp_info_task_handle[4];//1921,1922,1923,1924

/**
 * @ingroup xmb tcp server
 * Create a tcp server according to the configuration parameters
 * It is an internal function
 * @param Parameter the configuration parameters
 * @param k_server A netconn descriptor
 * @return ERR_OK if bound, any other err_t on failure
 */
esp_err_t CreateTcpServer(TcpInfoTaskHandleT *parameter) {

    while (parameter->k_server_ == NULL) {
        parameter->k_server_  = netconn_new(NETCONN_TCP);
    }
//    ESP_LOGI(TCP_TAG, "creat : %p", parameter->k_server_ );
    // netconn_set_nonblocking(conn, NETCONN_FLAG_NON_BLOCKING);
    //netconn_bind(*conn, &ip_info.ip, Param->port);
    /* Bind connection to well known port number 7. */
    ESP_ERROR_CHECK(netconn_bind(parameter->k_server_ , IP_ADDR_ANY, parameter->port_ ));
    ESP_ERROR_CHECK(netconn_listen(parameter->k_server_ )); /* Grab new connection. */
    ESP_LOGI(TCP_TAG,"PORT: %d LISTENING.....", parameter->port_);
    return ESP_OK;
}

/// TCP发送的任务
/// \param parameter 传参
_Noreturn void TcpServerSend(TcpInfoTaskHandleT *parameter) {
    ESP_LOGI(TCP_TAG, "Enter TcpServerSend...\n");

    QueueHandle_t *uart_to_tcp_queue = parameter->uart_tcp_queue_.uart_to_tcp_queue_;

    parameter->son_task_exists_ = true;
    events event;
    int ret = false;
    while (1) {
        ESP_LOGI(TCP_TAG, "TCP SENDING mode is working...\n");

        ret = xQueueReceive(*uart_to_tcp_queue, &event, portMAX_DELAY);
        if (ret == pdTRUE) {
            netconn_write(parameter->k_client_, event.buff_, event.buff_len_, NETCONN_NOCOPY);
            ESP_LOGI(TCP_TAG,"sending successfully!\n");
        }
        else{
            ESP_LOGE(TCP_TAG,"uart to tcp queue sending error!!\n");
        }
    }
    ESP_LOGE(TCP_TAG, "TcpServerSend task is deleted!!!\n");
    vTaskDelete(NULL);
}

/// TCP任务创建
/// \param parameter
/// \param priority
/// \return
esp_err_t TcpTaskCreate(TcpInfoTaskHandleT *parameter, int priority) {
    //printf("parameter uart_to_tcp_queue_:%p\n", parameter->UartTcpQueue.uart_to_tcp_queue_);
    //printf("parameter tcp_to_uart_queue_:%p\n", parameter->UartTcpQueue.tcp_to_uart_queue_);


    char pc_name[18];
    ESP_LOGI(TCP_TAG, "\nParam->mode:%d", parameter->mode_);
    switch (parameter->mode_) {
        case TCP_ALL:sprintf(pc_name, "TcpRec%d", parameter->port_);
            if (xTaskCreatePinnedToCore((TaskFunction_t) TcpServerAcceptWithRec,
                                    (const char *const) pc_name,
                                    5120,
                                    parameter,
                                    priority,
                                        parameter->k_father_task_,
                                        0) == pdPASS) {
                parameter->father_task_exists_ = true;
            }
            break;
        default:break;
    }
    return ESP_OK;
}

/// 删除TCP NetIF 所有句柄和任务
/// \param tcp_task_handle_delete
/// \return 删除是否成功
esp_err_t TcpTaskAllDelete(TcpInfoTaskHandleT *tcp_task_handle_delete) {

    if ((tcp_task_handle_delete->father_task_exists_) == true) {
        if (tcp_task_handle_delete->son_task_exists_ == true) {//判断子任务是否存在
            vTaskDelete(*(tcp_task_handle_delete->k_son_task_));
            tcp_task_handle_delete->son_task_exists_ = false;
            tcp_task_handle_delete->k_son_task_ = NULL;
            }

        netconn_delete(tcp_task_handle_delete->k_client_);
        netconn_delete(tcp_task_handle_delete->k_server_);
        tcp_task_handle_delete->k_client_ = NULL;
        tcp_task_handle_delete->k_server_ = NULL;
        vTaskDelete(*(tcp_task_handle_delete->k_father_task_));
        tcp_task_handle_delete->k_father_task_ = NULL;
        tcp_task_handle_delete->father_task_exists_ = false;

        tcp_task_handle_delete->mode_ = 0;


        }

    return ESP_OK;
}

/// TCP接受连接与接收的任务
/// \param parameter 传参
void TcpServerAcceptWithRec(TcpInfoTaskHandleT *parameter) {
    err_t err = ESP_OK;

    ESP_LOGI(TCP_TAG, "entering TcpServerAcceptWithRec with port %d", parameter->port_);
//    ESP_LOGI(TCP_TAG, "entering TcpServerAcceptWithRec with port %p", &(parameter->k_server_));
    ESP_ERROR_CHECK(CreateTcpServer(parameter));//创建TCP Server netCONN协议
//    ESP_LOGI(TCP_TAG, "creat : %p", parameter->k_server_);

    while (1) {
        int re_err;
        ESP_LOGI(TCP_TAG, "begin to accept:%d", parameter->port_);
        err = netconn_accept(parameter->k_server_, &parameter->k_client_);
        ESP_LOGI(TCP_TAG, "accepted from %d", parameter->port_);

        /* Process the new connection. */

        if (err == ERR_OK) {

            if (!parameter->son_task_exists_) {
                /*Create Receive Subtask*/
                //sprintf(tmp, "tcp_subtask_%d", parameter->port);
                //printf("\n%s\n", tmp);
                char tmp[16];
                sprintf(tmp, "TcpSend%d", parameter->port_);
                if (xTaskCreatePinnedToCore((TaskFunction_t) TcpServerSend,
                                            (const char *const) tmp,
                                            4096,
                                            (parameter),
                                            14,
                                            parameter->k_son_task_, 0)
                    == pdPASS) {
                    parameter->son_task_exists_ = true;
                    //strcpy(TCP_TASK_HANDLE[kTcpHandleFatherTaskCurrent].son_taskname_, tmp);
                }
                else{
                    ESP_LOGE(TCP_TAG,"tcp sending task is not built!\n");
                }
            }
            /*Create send buffer*/

            //发送工作
            void *data;
            struct netbuf *buf;
            events tx_event_;
            while (parameter->k_client_->state != NETCONN_CLOSE) {

                re_err = netconn_recv(parameter->k_client_, &buf);
                if (re_err == ERR_OK) {
                    do{

                        netbuf_data(buf, &data, &tx_event_.buff_len_);
                        tx_event_.buff_ = data;
//                        if (newconn_All->state != NETCONN_CLOSE) {
//                            netbuf_delete(buf);
//                            break;
//                        }
                        if (xQueueSend(*(parameter->uart_tcp_queue_.tcp_to_uart_queue_), &tx_event_, pdMS_TO_TICKS(10))
                            == pdPASS) {

                        }
                        else
                            ESP_LOGE(TCP_TAG, "TCP_SEND TO QUEUE FAILD\n");

                    }while ((netbuf_next(buf) >= 0));
                        netbuf_delete(buf);

                } else if (re_err == ERR_CLSD) {
                    ESP_LOGE(TCP_TAG, "DISCONNECT PORT:%d\n", parameter->port_);

                }
            }
            ESP_LOGE(TCP_TAG, "TcpServerAcceptWithRec task is deleted!!!\n");
            vTaskDelete(NULL);
        }
    }
    vTaskDelete(NULL);
}

[[maybe_unused]]
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