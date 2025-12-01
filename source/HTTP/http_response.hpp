#pragma once

#include"../tcpserver.hpp"

std::string deafult_version("HTTP/1.1");

struct HttpResponse
{
    int _status;
    std::string _version;
    std::string _body;
    bool _is_redirect;
    std::string _redirect_url;
    std::unordered_map<std::string, std::string> _headers;

    HttpResponse() : _status(200), _is_redirect(false), _version(deafult_version) {}
    HttpResponse(int status) : _status(status), _is_redirect(false) {}
    // 重置
    void reset()
    {
        _status = 200;
        _version.clear();
        _body.clear();
        _is_redirect = false;
        _redirect_url.clear();
        _headers.clear();
    }

    // 插入响应报头
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

    // 是否有响应报头
    bool has_header(const std::string& key)
    {
        auto it = _headers.find(key);
        if(it == _headers.end()) return false;
        return true;
    }

    // 获取响应报头
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

    // 设置重定向
    void set_redirect(const std::string& redirect_url, int status)
    {
        _status = status;
        _is_redirect = true;
        _redirect_url = redirect_url;
    }

    // 设置正文
    void set_body(const std::string& body, const std::string& content_type)
    {
        set_header("Content-Type", content_type);
        _body = body;
    }

    void set_version(const std::string& version)
    {
        _version = version;
    }

    // 是否是短连接
    bool is_short_connection()
    {
        if(has_header("Connection"))
        {
            if(get_header("Connection") == "close") return true;
            else return false;
        }
        else
        {
            if(_version == "HTTP/1.0") return true; // 1.0默认短连接
            else return false; // 1.1默认长连接
        }
    }
};