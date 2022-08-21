#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer {
public:
    //*参数：端口、触发模式、超时时间、是否优雅关闭、数据库配置、连接池数量、线程池数量、日志开关、日志等级、日志异步队列容量
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char* sqlUser, const  char* sqlPwd,
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();

    void Start();

private:
    bool InitSocket_();     //初始化套接字
    void InitEventMode_(int trigMode);  //初始化触发模式
    void AddClient_(int fd, sockaddr_in addr);  //添加客户端

    void DealListen_();     //处理监听事件
    void DealWrite_(HttpConn* client);  //处理写事件
    void DealRead_(HttpConn* client);   //处理读事件

    void SendError_(int fd, const char* info);  //发送错误信息
    void ExtentTime_(HttpConn* client); //
    void CloseConn_(HttpConn* client);  //关闭连接

    void OnRead_(HttpConn* client);     //正在读
    void OnWrite_(HttpConn* client);    //正在写
    void OnProcess(HttpConn* client);   //正在处理事件

    static const int MAX_FD = 65536;    //最大文件描述符
    static int SetFdNonBlock(int fd);   //设置文件非阻塞

    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;   //定时器
    std::unique_ptr<ThreadPool> threadpool_;    //线程池
    std::unique_ptr<Epoller> epoller_;   //epoll I/O多路复用
    std::unordered_map<int, HttpConn> users_;   //连接序号
};


#endif