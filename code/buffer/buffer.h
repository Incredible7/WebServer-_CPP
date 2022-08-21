#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>      //perror
#include <iostream>
#include <unistd.h>     //write
#include <sys/uio.h>    //readv
#include <vector>       //readv
#include <atomic>
#include <assert.h>

class Buffer{
public:
    Buffer(int initBuffersize = 1024);
    ~Buffer() = default;    //声明显示析构函数

    //缓冲区可写内存大小
    size_t WritableBytes() const;
    //缓冲区可读内存大小
    size_t ReadableBytes() const;
<<<<<<< HEAD
    //?缓冲区可前置（可读）内存位置
    size_t PrependableBytes() const;

    //?返回当前可读但未读的内存位置
=======
    //缓冲区读内存位置
    size_t PrependableBytes() const;

    //返回当前读指针
>>>>>>> f9bd2f2 (1)
    const char* Peek() const;
    //保证缓冲区对于len可写，不可写则更新缓冲区
    void EnsureWritable(size_t len);
    //更新writePos_
    void HasWritten(size_t len);

    //检索添加长度len后是否超出
    void Retrieve(size_t len);
    //检索当前是否超出end
    void RetrieveUntil(const char* end);
    //缓冲区重置
    void RetrieveAll();
    //当前位置取出所有可读数据并重置缓冲区
    std::string RetrieveAllToStr();

    //返回开始写数据的位置
    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
<<<<<<< HEAD
    //四个Append版本，实现缓冲区的拓展
=======
    //四个Append版本，实现缓冲区数据附加
>>>>>>> f9bd2f2 (1)
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
<<<<<<< HEAD
=======
    //拓展缓冲区空间
>>>>>>> f9bd2f2 (1)
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};


#endif