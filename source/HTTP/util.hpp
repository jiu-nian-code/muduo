#include"../tcpserver.hpp"

struct Util
{
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
    static void read_file()
    {

    }

    // 文件写入
    // url编码
    // url解码
    // 状态码的描述信息获取
    // 根据文件后缀分析文件mime
    // 判断一个文件是否是一个目录
    // 判断一个文件是否是一个普通文件
    // 资源路径是否有效是否违法
};