#include"../tcpserver.hpp"

#include<regex>

std::string default_version = "HTTP/1.1";

class HttpRequest
{
    std::string _method; // 请求方法
    std::string _path; // 请求路径
    std::string _version; // 协议版本
    std::string _body; // 请求正文
    std::smatch _matches; // 正则表达式的提取数据
    std::unordered_map<std::string, std::string> _headers; // 请求字段
    std::unordered_map<std::string, std::string> _params; // 查询字符串
public:
    HttpRequest(const std::string& version = default_version) : _version(version) {}

    void reset()
    {
        _method.clear();
        _path.clear();
        _version.clear();
        _body.clear();
        std::smatch matches;
        _matches.swap(matches);
        _headers.clear();
        _params.clear();
    }

    // 插入头部字段
    bool set_header(const std::string& key, const std::string& value)
    {
        auto it = _headers.find(key);
        if(it != _headers.end())
        {
            DBG_LOG("key exist.");
            return false;
        }
        _headers.insert(make_pair(key, value));
        return true;
    }

    // 是否有头部字段
    bool has_header(const std::string& key)
    {
        auto it = _headers.find(key);
        if(it == _headers.end()) return false;
        return true;
    }

    // 获取头部字段
    std::string get_header(const std::string& key)
    {
        auto it = _headers.find(key);
        if(it == _headers.end())
        {
            DBG_LOG("key is not exist.");
            return std::string();
        }
        return it->second;
    }

    // 插入查询字符串
    bool set__param(const std::string& key, const std::string& value)
    {
        auto it = _params.find(key);
        if(it != _params.end())
        {
            DBG_LOG("key exist.");
            return false;
        }
        _params.insert(make_pair(key, value));
        return true;
    }

    // 是否有查询字符串
    bool has__param(const std::string& key)
    {
        auto it = _params.find(key);
        if(it == _params.end()) return false;
        return true;
    }

    // 获取头部字符串
    std::string get__param(const std::string& key)
    {
        auto it = _params.find(key);
        if(it == _params.end())
        {
            DBG_LOG("key is not exist.");
            return std::string();
        }
        return it->second;
    }

    // 获取正文长度
    int content_length()
    {
        if(has_header("Content-Length"))
        {
            std::string length = _headers["Content-Length"];
            return std::stoi(length);
        }
        return 0;
    }
    
    // 判断是否是短连接
    bool is_short_connection()
    {
        if(has_header("Connection"))
        {
            if(get_header("Connection") == "close") return true;
            else return false;
        }
        else
        {
            if(_version == "HTTP/1.0") return true;
            else return false;
        }
    }
};