#pragma once

#include"../tcpserver.hpp"

#include"util.hpp"

#include"http_request.hpp"

#define HTTP_LINE_MAX 8196

#define HTTP_HEADER_MAX 4096

typedef enum
{
    RECV_HTTP_ERROR,
    RECV_HTTP_LINE,
    RECV_HTTP_HEAD,
    RECV_HTTP_BODY,
    RECV_HTTP_OVER
} HttpRecvStatu;

class HttpContext
{
    HttpRequest _request;
    HttpRecvStatu _recv_stu;
    int _resp_statu;

    bool recv_line(Buffer& buf)
    {
        if(_recv_stu != RECV_HTTP_LINE) return false;
        std::string line = buf.GetLine();
        if(line.size() == 0)
        {
            if(buf.effective_read_area() > HTTP_LINE_MAX) // 请求行太长出错返回
            {
                _resp_statu = 414; // URI Too Long
                _recv_stu = RECV_HTTP_ERROR;
                return false;
            }
            // 请求行没读完, 继续读
            return true;
        }
        if(line.size() > HTTP_LINE_MAX) // 请求行太长出错返回
        {
            _resp_statu = 414;
            _recv_stu = RECV_HTTP_ERROR;
            return false;
        }

        bool ret = Pars_line(line);
        if(!ret) return false;
        _resp_statu = RECV_HTTP_HEAD;
        return true;
    }

    bool Pars_line(const std::string& line)
    {
        std::regex e("(GET|POST|PUT|HEAD|DELETE|OPTIONS|TRACE|CONNECT|LINK|UNLINK) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?", std::regex::icase); // std::regex::icase不区分大小写
        std::smatch matchs;
        bool ret = std::regex_match(line, matchs, e);
        if(!ret)
        {
            _resp_statu = 400; // BAD REQUEST
            _recv_stu = RECV_HTTP_ERROR;
            return false;
        }

        _request._method = matchs[1]; // 不需要解码, 浏览器加的
        std::transform(_request._method.begin(), _request._method.end(), _request._method.begin(), ::toupper);
        _request._path = Util::url_decodeing(matchs[2], false); // 不需要空格转换
        _request._version = matchs[4];// 不需要解码, 浏览器加的

        std::vector<std::string> output;
        Util::str_split(matchs[3], "&", output);
        for(auto& e : output)
        {
            size_t pos = e.find("=");
            if(pos == std::string::npos)
            {
                _resp_statu = 400;
                _recv_stu = RECV_HTTP_ERROR;
                return false;
            }
            std::string key = Util::url_decodeing(e.substr(0, pos), true); // 只有查询字符串部分才需要空格转+
            std::string value = Util::url_decodeing(e.substr(pos + 1), true);
            _request.set_param(key, value);
        }
        return true;
    }

    bool recv_header(Buffer& buf)
    {
        if(_recv_stu != RECV_HTTP_HEAD) return false;
        while(1)
        {
            std::string line = buf.GetLine();
            if(line.size() == 0)
            {
                if(buf.effective_read_area() > HTTP_HEADER_MAX) // 请求行太长出错返回
                {
                    _resp_statu = 414; // URI Too Long
                    _recv_stu = RECV_HTTP_ERROR;
                    return false;
                }
                // 请求行没读完, 继续读
                return true;
            }
            if(line == "\n" || line == "\r\n") break;
            if(line.size() > HTTP_HEADER_MAX) // 请求行太长出错返回
            {
                _resp_statu = 414;
                _recv_stu = RECV_HTTP_ERROR;
                return false;
            }
            Pars_header(line);
        }
        _recv_stu = RECV_HTTP_BODY;
        return true;
    }

    bool Pars_header(std::string line)
    {
        int gap = 1;
        if(line.back() == '\n') line.pop_back();
        if(line.back() == '\r') ++gap, line.pop_back();
        size_t pos = line.find("= ");
        if(pos == std::string::npos)
        {
            _resp_statu = 400;
            _recv_stu = RECV_HTTP_ERROR;
            return false;
        }
        _request.set_header(line.substr(0, pos), line.substr(pos + gap));
        return true;
    }

    bool recv_body(Buffer& buf)
    {
        if(_recv_stu != RECV_HTTP_BODY) return false;
        if(_request.content_length() == 0) return true; // 正文长度为0, 直接返回
        size_t len = _request.content_length() - _request._body.size();
        if(buf.effective_read_area() >= len) // 缓冲区数据够
        {
            _request._body += buf.read_as_string(len);
            _resp_statu = RECV_HTTP_OVER;
            return true;
        }
        _request._body += buf.read_as_string(buf.effective_read_area());
        return true;
    }
public:
    HttpContext() :  _recv_stu(RECV_HTTP_LINE), _resp_statu(200) {}

    void reset()
    {
        _recv_stu = RECV_HTTP_LINE;
        _resp_statu = 200;
        _request.reset();
    }

    int RespStatu() { return _resp_statu; }

    HttpRecvStatu RecvStatu() { return _recv_stu; }

    HttpRequest &Request() { return _request; }

    void RecvHttpRequest(Buffer& buf)
    {
        switch (_recv_stu)
        {
            case RECV_HTTP_LINE : recv_line(buf);
            case RECV_HTTP_HEAD : recv_header(buf);
            case RECV_HTTP_BODY : recv_body(buf);
        }
        return;
    }
};