#include"../tcpserver.hpp"

#include<fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class Util
{
    static std::unordered_map<int, std::string> httpStatusMap;

    static std::unordered_map<std::string, std::string> suffix_to_mime;

    static bool is_reserved_characters(char e)
    {
        return (e >= '0' && e <= '9') ||
        (e >= 'A' && e <= 'Z') ||
        (e >= 'a' && e <= 'z') ||
        e == '-' || e == '_' ||
        e == '.' || e == '~';
    }

    static char HEXtoCHAR(char a)
    {
        char re = 0;
        if(a >= '0' && a <= '9')
            re += (a - '0');
        else if(a>= 'A' && a <= 'F')
            re += (a - 'A' + 10);
        else if(a >= 'a' && a <= 'f')
            re += (a - 'a' + 10);
        return re;
    }

    static bool valid_HEX(char a)
    {
        if((a >= '0' && a <= '9') || (a >= 'A' && a <= 'F') || (a >= 'a' && a <= 'f')) return true;
        return false;
    }

public:
    // 字符串分割
    static void str_split(const std::string& str, const std::string& inter, std::vector<std::string>& output)
    {
        size_t pos = 0;
        size_t gap = inter.size();
        while(pos < str.size())
        {
            size_t ret = str.find(inter, pos);
            if(ret == std::string::npos)
            {
                output.push_back(str.substr(pos));
                break;
            }
            if(ret == pos) { pos += gap; continue; }
            output.push_back(str.substr(pos, ret - pos));
            pos = ret + gap;
        }
    }

    // 文件读取
    static bool read_file(const std::string& filename, Buffer& buf)
    {
        std::ifstream ifs(filename, std::ios::binary);
        if(!ifs.is_open())
        {
            ERR_LOG("file open fail.");
            return false;
        }
        ifs.seekg(0, ifs.end);
        size_t sz = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        buf.resize(sz);
        ifs.read(buf.write_position(), sz);
        if(!ifs.good())
        {
            ERR_LOG("read file fail.");
            return false;
        }
        buf.Move_Write_Loc(ifs.gcount());
        ifs.close();
        return true;
    }

    // 文件写入
    static bool write_file(const std::string& filename, Buffer& buf)
    {
        std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
        if(!ofs.is_open())
        {
            ERR_LOG("file open fail.");
            return false;
        }
        std::streampos pos = ofs.tellp();
        ofs.write(buf.read_position(), buf.effective_read_area());
        if(!ofs.good())
        {
            ERR_LOG("write file fail.");
            return false;
        }
        std::streampos afterpos = ofs.tellp();
        size_t sz = afterpos - pos;
        buf.Move_Read_Loc(sz);
        return true;
    }

    // url编码
    // A-Z, a-z, 0-9, -, _, ., ~ 保持不变
    static std::string url_encoding(const std::string& str, bool is_space_to_plus)
    {
        std::string ret;
        for(auto& e : str)
        {
            if(is_reserved_characters(e))
                ret += e;
            else if(e == ' ' && is_space_to_plus) ret += '+';
            else
            {
                char arr[4];
                snprintf(arr, sizeof(arr), "%%%02X", e);
                ret += arr;
            }
        }
        return ret;
    }

    // url解码
    static std::string url_decodeing(const std::string& str, bool is_space_to_plus)
    {
        std::string ret;
        int sz = str.size();
        for(int i = 0; i < sz; ++i)
        {
            char e = str[i];
            if(is_reserved_characters(e))
                ret += e;
            else if(e == '+' && is_space_to_plus) ret += ' ';
            else if(e == '%')
            {
                char x = str[i + 1];
                char y = str[i + 2];
                if(!(valid_HEX(x) && valid_HEX(y)))
                {
                    ERR_LOG("url is invalid.");
                    return std::string();
                }
                char a = HEXtoCHAR(x);
                char b = HEXtoCHAR(y);
                ret += (a * 16 + b);
                // ret += a << 4 + b;
            }
        }
        return ret;
    }

    // 状态码的描述信息获取
    static std::string get_status_message(int stu_msg)
    {
        auto it = httpStatusMap.find(stu_msg);
        if(it == httpStatusMap.end()) return std::string("Unknow.");
        return it->second;
    }

    // 根据文件后缀分析文件mime
    static std::string get_file_mime(const std::string& filename)
    {
        size_t pos = filename.find_last_of('.');
        if(pos == std::string::npos)
            return "application/octet-stream";
        std::string extension = filename.substr(pos);
        auto it = suffix_to_mime.find(extension);
        if(it == suffix_to_mime.end())
            return "application/octet-stream";
        return it->second;
    }

    // 判断一个文件是否是一个目录
    static bool is_directory(const std::string& filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISDIR(st.st_mode);
    }

    // 判断一个文件是否是一个普通文件
    static bool is_regular(const std::string& filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISREG(st.st_mode);
    }

    // 资源路径是否有效是否违法
    static bool is_valid_path(const std::string& filename)
    {
        // static void str_split(const std::string& str, const std::string& inter, std::vector<std::string>& output)
        std::vector<std::string> arr;
        str_split(filename, "/", arr);
        int level = 0;
        for(auto& e : arr)
        {
            if(e == "..") --level;
            else if(e != ".") ++level;
            if(level < 0) return false;
        }
        return true;
    }
};

std::unordered_map<int, std::string> Util::httpStatusMap = 
{
    // 1xx: Informational
    {100, "Continue"},
    {101, "Switching Protocols"},
    {102, "Processing"},

    // 2xx: Success
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},

    // 3xx: Redirection
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},

    // 4xx: Client Error
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"}, // RFC 2324
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},

    // 5xx: Server Error
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}
};

std::unordered_map<std::string, std::string> Util::suffix_to_mime = 
{
    // 文本与文档类型
    {".txt", "text/plain"},
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".csv", "text/csv"},
    {".pdf", "application/pdf"},
    {".rtf", "application/rtf"},
    {".md", "text/markdown"},
    {".php", "text/x-php"},

    // 办公文档类型 (旧版与新版的Microsoft Office、WPS等)
    {".doc", "application/msword"},
    {".dot", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".pps", "application/vnd.ms-powerpoint"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".wps", "application/vnd.ms-works"},

    // 图片类型
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".bmp", "image/bmp"},
    {".svg", "image/svg+xml"},
    {".webp", "image/webp"},
    {".ico", "image/x-icon"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".psd", "image/vnd.adobe.photoshop"},

    // 音频类型
    {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},
    {".ogg", "audio/ogg"},
    {".mid", "audio/midi"},
    {".aac", "audio/aac"},
    {".flac", "audio/flac"},
    {".wma", "audio/x-ms-wma"},

    // 视频类型
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".mov", "video/quicktime"},
    {".flv", "video/x-flv"},
    {".mkv", "video/x-matroska"},
    {".webm", "video/webm"},
    {".wm", "video/x-ms-wmv"},

    // 压缩包类型
    {".zip", "application/zip"},
    {".rar", "application/vnd.rar"},
    {".7z", "application/x-7z-compressed"},
    {".tar", "application/x-tar"},
    {".gz", "application/gzip"},
    {".bz2", "application/x-bzip2"},
    {".tgz", "application/x-gzip"},

    // 字体类型
    {".ttf", "font/ttf"},
    {".otf", "font/otf"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},

    // 应用程序与可执行文件类型
    {".exe", "application/octet-stream"},
    {".dll", "application/octet-stream"},
    {".apk", "application/vnd.android.package-archive"},
    {".deb", "application/vnd.debian.binary-package"},
    {".rpm", "application/x-rpm"},
    {".msi", "application/x-msi"},

    // 编程语言源文件及其他文本类型
    {".py", "text/x-python"},
    {".java", "text/x-java-source"},
    {".c", "text/x-c"},
    {".cpp", "text/x-c++"},
    {".h", "text/x-c"},
    {".cs", "text/x-csharp"},
    {".rb", "text/x-ruby"},
    {".pl", "text/x-perl"},
    {".sh", "text/x-shellscript"},
    {".sql", "application/sql"}
};