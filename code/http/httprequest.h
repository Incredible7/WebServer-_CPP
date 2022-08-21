#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

using std::string;

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };

    enum HTTP_CODE  {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTIONM
    };

    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);

    string path() const;
    string& path();
    string method() const;
    string version() const;
    string GetPost(const string& key) const;
    string GetPost(const char* key) const;

    bool IsKeepAlive() const;

    //todo void HttpConn::ParseFormData() {} void HttpConn::ParseJson() {}
private:
    //解析请求行
    bool ParseRequestLine_(const string& line);
    //解析请求头
    void ParseHeader_(const string& line);
    //解析请求体
    void ParseBody_(const string& line);

    //解析路径
    void ParsePath_();
    //解析post
    void ParsePost_();
    //解析解码类型
    void ParseFromUrlencoded_();

    //用户验证
    static bool UserVerify(const string& name, const string& pwd, bool isLogin);

    PARSE_STATE state_;
    string method_, path_, version_, body_;
    std::unordered_map<string, string> header_;
    std::unordered_map<string, string> post_;

    static const std::unordered_set<string> DEFAULT_HTML;
    static const std::unordered_map<string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);
};


#endif