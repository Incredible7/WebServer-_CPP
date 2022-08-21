#include "webserver.h"

using namespace std;

WebServer::WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize):
        port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
        timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
    {
        srcDir_ = getcwd(nullptr, 256); //获取当前工程主目录
        assert(srcDir_);                //判断目录是否存在
        strncat(srcDir_, "/resources/", 16); //修改路径为网页资源路径
        HttpConn::userCount = 0;        //用户数量初始化为零
        HttpConn::srcDir = srcDir_;     //统一路径
        SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
        //初始化数据库案例
        InitEventMode_(trigMode);       //设置触发模式
        if(!InitSocket_()) { isClose_ = true; } //初始化失败，关闭套接字
        
        //日志开启，初始化日志文件
        if(openLog){
            Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
            if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
            else{
                LOG_INFO("========== Server init ==========");
                LOG_INFO("Port: %d, OpenLinger: %s", port_, OptLinger? "true" : "false");
                LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                        (listenEvent_ & EPOLLET ? "ET" : "LT"),
                        (connEvent_ & EPOLLET ? "ET" : "LT"));
                LOG_INFO("LogSys level: %d", logLevel);
                LOG_INFO("srcDir: %s", HttpConn::srcDir);
                LOG_INFO("SqlConnPool Num: %d, ThreadPool Num: %d", connPoolNum, threadNum);
            }
        }
    }

WebServer::~WebServer(){
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode){
<<<<<<< HEAD
    listenEvent_ = EPOLLRDHUP;
=======
    listenEvent_ = EPOLLRDHUP;          //对端连接断开触发的 epoll 事件会包含 EPOLLIN | EPOLLRDHUP
>>>>>>> f9bd2f2 (1)
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::Start(){
    int timeMS = -1;    //无事件将阻碍
    if(!isClose_){ LOG_INFO("========== Server start =========="); }
    while(!isClose_){
        if(timeoutMS_ > 0){
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);  //返回事件处理数
        for(int i = 0; i < eventCnt; ++i){
            //处理事件
            int fd = epoller_->GetEventFd(i);   //获得事件描述符
            uint32_t events = epoller_->GetEvents(i);   //获得事件
            if(fd == listenFd_){                //事件匹配开始处理
                DealListen_();
            }
            //事件未匹配并且事件挂起或错误则关闭
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            //事件正在读，处理读
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            //事件正在写，处理写
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }
            //事件异常，未知事件类型
            else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char* info){
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0){ LOG_WARN("send error to client[%d] error!", fd); }
    close(fd);
}

void WebServer::CloseConn_(HttpConn* client){
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient_(int fd, sockaddr_in addr){
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0){
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonBlock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::DealListen_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(listenFd_, (struct sockaddr*)& addr, & len);
        if(fd <= 0){ return; }
        else if(HttpConn::userCount >= MAX_FD){
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return ;
        }
        AddClient_(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn* client){
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtentTime_(HttpConn* client){
    assert(client);
    if(timeoutMS_ > 0){ timer_->adjust(client->GetFd(), timeoutMS_); }
}

void WebServer::OnRead_(HttpConn* client){
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN){
        CloseConn_(client);
        return ;
    }
    OnProcess(client);
}

void WebServer::OnProcess(HttpConn* client){
    if(client->process()){
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else{
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::OnWrite_(HttpConn* client){
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0){
        //传输结束
        if(client->IsKeepAlive()){
            OnProcess(client);
            return ;
        }
    } else if(ret < 0){
        if(writeErrno == EAGAIN){
            //继续传输
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return ;
        }
    }
    CloseConn_(client);
}

//创建监听事件
bool WebServer::InitSocket_(){
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024){
        std::cout <<" 1 " << std::endl;
        LOG_ERROR("Port: %d error!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger = { 0 };
    if(openLinger_){
        /*优雅关闭：直到所剩数据发送完毕或超时
        close()不会立刻返回，内核会延迟一段时间，这个时间就由l_linger的值来决定。
        如果超时时间到达之前，发送完未发送的数据(包括FIN包)并得到另一端的确认，close()会返回正确，
        socket描述符优雅性退出。否则，close()会直接返回错误值，未发送数据丢失，socket描述符被强制性退出。
        需要注意的是，如果socket描述符被设置为非堵塞型，则close()会直接返回值。
        */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0){
        std::cout <<" 2 " << std::endl;
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    //设置套接字选项
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0){
        std::cout <<" 3 " << std::endl;
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    //端口复用，只有最后一个套接字会正常接收数据
    //开启地址复用
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)& optval, sizeof(int));
    if(ret == -1) {
        std::cout <<" 4 " << std::endl;
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        std::cout <<" 5 " << std::endl;
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        std::cout <<" 6 " << std::endl;
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        std::cout <<" 7 " << std::endl;
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }

    SetFdNonBlock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

//给文件附加非阻塞的描述
int WebServer::SetFdNonBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}