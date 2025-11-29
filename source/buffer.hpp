#pragma once

#include<iostream>

#include<vector>

#include<algorithm>

#include<string.h>

#include"log.hpp"

#define DEFAULT_BUFFER_SIZE 10

// TODO : 可以使用环形数组实现

class Buffer
{
    typedef Buffer Self;
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

    bool directly_write(const char* source, size_t sz)
    {
        for(int i = 0; i < sz; ++i)
        {
            _arr[_write_loc + i] = source[i];
        }
        _write_loc += sz;

        return true;
    }

    void expansion(size_t sz)
    {
        size_t old_sz = _arr.size();
        size_t new_sz = old_sz;
        size_t era = effective_read_area();
        while(new_sz - era < sz)
        {
            new_sz *= 2;
        }
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

        return;
    }

    size_t Head()
    {
        return _read_loc;
    }

    size_t Tail()
    {
        return _arr.size() - _write_loc;
    }

    bool directly_read(char* destination, size_t sz)
    {
        for(int i = 0; i < sz; ++i, ++_read_loc)
        {
            destination[i] = _arr[_read_loc];
        }

        return true;
    }

    bool makesure_writearea(size_t sz)
    {
        size_t head = Head();
        size_t tail = Tail();
        size_t ewa = effective_write_area();
        if(tail < sz && ewa >= sz) // 对齐
        {
            align();
        }
        else // 扩容
        {
            expansion(sz);
        }
        return true;
    }

    char* Begin()
    {
        return &(*(_arr.begin()));
    }

    char* FindCRLF()
    {
        // void *memchr(const void *s, int c, size_t n);
        char* res = (char*)memchr(read_position(), '\n', effective_read_area());
        return res;
    }

public:
    Buffer():
        _arr(DEFAULT_BUFFER_SIZE),
        _read_loc(0),
        _write_loc(0)
    {}

    void resize(size_t sz)
    {
        expansion(sz);
    }

    size_t effective_write_area()
    {
        return Head() + Tail();
    }

    size_t effective_read_area()
    {
        return _write_loc - _read_loc;
    }

    char* write_position()
    {
        return Begin() + _write_loc;
    }

    char* read_position()
    {
        return Begin() + _read_loc;
    }

    bool wirte(const char* source, size_t sz)
    {
        return makesure_writearea(sz) && directly_write(source, sz);
    }

    bool write_buffer(Self& source) // 不加const
    {
        return wirte(source.read_position(), source.Size());
    }

    bool write_string(std::string& str)
    {
        return wirte(str.c_str(), str.size());
    }

    std::string read_as_string(size_t sz)
    {
        std::string str;
        str.resize(sz);
        ssize_t ret = read(&(*(str.begin())), sz);
        if(ret > 0) return str;
        return std::string();
    }

    ssize_t read(char* destination, size_t sz)
    {
        // size_t era = effective_read_area();
        sz = std::min(effective_read_area(), sz);
        if(directly_read(destination, sz)) return sz;
        return -1;
    }

    void Move_Read_Loc(size_t len) // danger
    {
        if(len > effective_read_area())
        {
            ERR_LOG("move error, len is longer than effective_read_area");
        }
        _read_loc += len;
    }

    void Move_Write_Loc(size_t len) // danger, make sure buffer can directly write.
    {
        if(len > effective_write_area())
        {
            ERR_LOG("move error, len is longer than effective_write_area");
        }
        _write_loc += len;
        //_write_loc %= _arr.size(); // 其实可以不用加
    }


    ssize_t GetLine(char* destination)
    {
        char* res = FindCRLF();
        return read(destination, res - read_position() + 1);
    }

    size_t Size()
    {
        return effective_read_area();
    }

    void Clear()
    {
        _read_loc = 0;
        _write_loc = 0;
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