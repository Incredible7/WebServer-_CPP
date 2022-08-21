#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
#include "../buffer/buffer.h"
#include "../log/log.h"

using std::string;

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const string& srcDir, string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer& buff);
    void AddHeader_(Buffer& buff);
    void AddContent_(Buffer& buff);

    void ErrorHtml_();
    string GetFileType_();

    int code_;
    bool isKeepAlive_;

    string path_;
    string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<string, string> SUFFIX_TYPE;
    static const std::unordered_map<int, string> CODE_STATUS;
    static const std::unordered_map<int, string> CODE_PATH;
};


#endif