#include<iostream>

#include<vector>

#include<algorithm>

#define DEFAULT_BUFFER_SIZE 10

// TODO : 可以使用环形数组实现

class buffer
{
    std::vector<char> _arr;
    size_t _read_loc;
    size_t _write_loc;

    bool align()
    {
        int source = _read_loc;
        int destination = 0;
        while(source < _write_loc)
        {
            _arr[destination++] = _arr[source++];
        }
        _read_loc = 0;
        _write_loc = destination;

        return true;
    }

    bool directly_wirte(const char* source, size_t sz)
    {
        for(int i = 0; i < sz; ++i)
        {
            _arr[_write_loc + i] = source[i];
        }
        _write_loc += sz;

        return true;
    }

    bool expansion(size_t sz)
    {
        // std::cout << 1 << std::endl;
        size_t old_sz = _arr.size();
        size_t new_sz = old_sz;
        size_t era = effective_read_area();
        while(new_sz - era < sz)
        {
            new_sz *= 2;
        }
        // std::cout <<  old_sz << std::endl;
        // std::cout <<  sz << std::endl;
        // std::cout <<  era << std::endl;
        // std::cout <<  new_sz << std::endl;
        std::vector<char> tmp(new_sz);
        int source = _read_loc;
        int destination = 0;
        while(source < _write_loc)
        {
            tmp[destination++] = _arr[source++];
        }
        _arr.swap(tmp);
        _read_loc = 0;
        _write_loc = destination;

        return true;
    }

    size_t Head()
    {
        return _read_loc;
    }

    size_t Tail()
    {
        return _arr.size() - _write_loc;
    }

    size_t effective_write_area()
    {
        return Head() + Tail();
    }

    size_t effective_read_area()
    {
        return _write_loc - _read_loc;
    }

    bool directly_read(char* destination, size_t sz)
    {
        for(int i = 0; i < sz; ++i, ++_read_loc)
        {
            destination[i] = _arr[_read_loc];
        }

        return true;
    }

    bool makesure_wirtearea()
    {

    }

    bool makesure_readarea()
    {

    }

public:
    buffer():
        _arr(DEFAULT_BUFFER_SIZE),
        _read_loc(0),
        _write_loc(0)
    {}

    bool wirte(const char* source, size_t sz)
    {
        size_t head = Head();
        size_t tail = Tail();
        size_t ewa = effective_write_area();
        // std::cout << "head: " << head << " " << "tail: " << tail << " " << "ewa: " << ewa << std::endl;
        if(tail >= sz) // 直接写入
        {
            directly_wirte(source, sz);
        }
        else if(tail < sz && ewa >= sz) // 对齐后写入
        {
            align();
            directly_wirte(source, sz);
        }
        else // 扩容后写入
        {
            // std::cout << 1 << std::endl;
            expansion(sz);
            directly_wirte(source, sz);
        }

        return true;
    }

    bool read(char* destination, size_t sz)
    {
        // size_t era = effective_read_area();
        sz = std::min(effective_read_area(), sz);
        return directly_read(destination, sz);
    }

    void PRINT() // for debug
    {
        int cur = _read_loc;
        while(cur < _write_loc)
        {
            std::cout << _arr[cur++];
        }
        std::cout << std::endl << "_read_loc: " << _read_loc << " " << "_write_loc: " << _write_loc << std::endl;
    }
};

// int main()
// {
//     buffer buf;
//     std::string str("hello world");
//     buf.wirte(str.c_str(), str.size());
//     buf.wirte(str.c_str(), str.size());
//     buf.wirte(str.c_str(), str.size());
//     buf.wirte(str.c_str(), str.size());
//     buf.wirte(str.c_str(), str.size());
//     buf.wirte(str.c_str(), str.size());

//     char arry[1024] = {0};
//     buf.read(arry, 1024);

//     std::string str1("hello linux");
//     buf.wirte(str1.c_str(), str.size());
//     buf.wirte(str1.c_str(), str.size());
//     // buf.wirte(str1.c_str(), str.size());
//     // buf.wirte(str1.c_str(), str.size());
//     // buf.wirte(str1.c_str(), str.size());
//     // buf.wirte(str1.c_str(), str.size());
//     buf.PRINT ();

//     std::cout << "------------------------------------------" << std::endl;
//     std::cout << arry << std::endl;
//     return 0;
// }