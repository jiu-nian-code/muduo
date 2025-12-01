#pragma once
#include"../tcpserver.hpp"
#include"http_request.hpp"
#include"http_response.hpp"
#include"http_context.hpp"
#include"util.hpp"

class HttpServer
{
    using handler = std::function<void(HttpRequest&, HttpResponse&)>;
    using handlers = std::vector<std::pair<std::regex, handler>>;
    handlers _get_route;
    handlers _post_route;
    handlers _put_route;
    handlers _delete_route;
    std::string _root_dir;
    Tcpserver _tcpserver;
    void headler_error(HttpResponse& resp)
    {
        std::string err_body;
        err_body += "<html>";
        err_body += "<head>";
        err_body += "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">";
        err_body += "</head>";
        err_body += "<body>";
        err_body += "<h1 aria-setsize=\"10\">";
        err_body += std::to_string(resp._status);
        err_body += " ";
        err_body += Util::get_status_message(resp._status);
        err_body += "</h1>";
        err_body += "</body>";
        err_body += "</html>";
        resp.set_body(err_body, Util::get_file_mime(".html"));
    }

    void write_reponse(const connect_ptr& con_ptr, HttpRequest& req, HttpResponse& resp)
    {
        if(!req._version.empty()) resp.set_version(req._version);
        if(req.is_short_connection()) resp.set_header("Connection", "close");
        else resp.set_header("Connection", "keep-alive");
        if(!resp._body.empty() && !resp.has_header("Content-Length"))
            resp.set_header("Content-Length", std::to_string(resp._body.size()));
        if(!resp._body.empty() && !resp.has_header("Content-Type"))
            resp.set_header("Content-Type", "application/octet-stream");

        std::stringstream reponse;
        reponse << resp._version << " " << resp._status << " " << Util::get_status_message(resp._status) << "\r\n";
        for(auto& e : resp._headers)
            reponse << e.first << ": " << e.second << "\r\n";
        reponse << "\r\n";
        reponse << resp._body;
        con_ptr->Send(reponse.str().c_str(), reponse.str().size());
    }

    bool is_headler_file(HttpRequest& req)
    {
        if(_root_dir.empty()) return false;
        if(req._method != "GET" && req._method != "PUT") return false;
        if(!Util::is_valid_path(req._path)) return false;

        std::string path = _root_dir + req._path;
        if(req._path == "/") path += "index.html";

        if(!Util::is_regular(path)) return false;
        return true;
    }

    void headler_file(HttpRequest& req, HttpResponse& resp)
    {
        std::string path = _root_dir + req._path;
        if(req._path == "/") path += "index.html";
        Buffer buf;
        Util::read_file(path, buf);
        resp.set_body(buf.read_as_string(buf.effective_read_area()), Util::get_file_mime(path));
    }

    bool dispatcher(handlers& hls, HttpRequest& req, HttpResponse& resp)
    {
        for(auto& e : hls)
        {
            if(std::regex_match(req._path, req._matches, e.first))
            {
                e.second(req, resp);
                return true;
            }
        }
        return false;
    }

    bool route(HttpRequest& req, HttpResponse& resp)
    {
        if(is_headler_file(req)) headler_file(req, resp);

        if(req._method == "GET" || req._method == "HEAD") return dispatcher(_get_route, req, resp);
        else if(req._method == "POST") return dispatcher(_post_route, req, resp);
        else if(req._method == "PUT") return dispatcher(_put_route, req, resp);
        else if(req._method == "DELETE") return dispatcher(_delete_route, req, resp);
        else { resp._status = 405; return false; }
    }

    void On_Connection(const connect_ptr& con_ptr)
    {
        con_ptr->SetContext(HttpContext());
        DBG_LOG("New Connection %p", con_ptr.get());
    }

    void On_Message(const connect_ptr& con_ptr, Buffer* buf)
    {
        while(1)
        {
            HttpContext* context = (con_ptr->GetContext())->Get_Value<HttpContext>();
            context->RecvHttpRequest(*buf);
            HttpRequest req = context->Request();
            HttpResponse resp(context->RespStatu());
            bool ret_route = true;
            if(context->RecvStatu() == RECV_HTTP_OVER) ret_route = route(req, resp);
            if(context->RecvStatu() == RECV_HTTP_ERROR || !ret_route) // context->RecvStatu() == RECV_HTTP_ERROR可以换context->RespStatu() >= 400
            {
                headler_error(resp);
                write_reponse(con_ptr, req, resp);
                context->reset();
                buf->Clear();
                con_ptr->Shutdown();
                return;
            }
            if(context->RecvStatu() != RECV_HTTP_OVER) return;
            write_reponse(con_ptr, req, resp);
            context->reset();
            if(req.is_short_connection()) break;
        }
    }
public:
    HttpServer(uint16_t port = DEFAULT_PORT, const std::string& ip = default_ip) :
        _tcpserver(port, ip)
    {
        _tcpserver.Set_Message_Callback(std::bind(&HttpServer::On_Message, this, std::placeholders::_1, std::placeholders::_2));
        _tcpserver.Set_Connected_Callback(std::bind(&HttpServer::On_Connection, this, std::placeholders::_1));
    }

    void set_root_dir(const std::string& dir)
    {
        if(Util::is_directory(dir))
        {
            ERR_LOG("dir error.");
            return;
        }
        _root_dir = dir;
    }

    void set_timeout(int timeout) { _tcpserver.Start_Inactive_Destruction(timeout); }
    void set_thread_num(int num) { _tcpserver.Set_Threadloop_Num(num); }
    void Get(const std::string& pattern, const handler& hl) { _get_route.push_back(make_pair(std::regex(pattern), hl)); }
    void POST(const std::string& pattern, const handler& hl) { _get_route.push_back(make_pair(std::regex(pattern), hl)); }
    void PUT(const std::string& pattern, const handler& hl) { _get_route.push_back(make_pair(std::regex(pattern), hl)); }
    void DELETE(const std::string& pattern, const handler& hl) { _get_route.push_back(make_pair(std::regex(pattern), hl)); }
    void Start() { _tcpserver.Start(); }
};