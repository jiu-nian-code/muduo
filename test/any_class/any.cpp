#include<iostream>

#include<string>

#include<typeinfo>

#include<typeindex>

#include<any>

class Any
{
    struct holder
    {
         virtual ~holder(){}; // 析构函数不能是纯虚函数

         virtual holder* clone() = 0;

         virtual const std::type_info& type() = 0;
    };

    template<class T>
    struct placeholder : public holder
    {
        placeholder(const T& con): _content(con) {}

        placeholder* clone() { return new placeholder<T>(_content); }

        const std::type_info& type() { return typeid(T); }

        T _content;
    };

    holder* _ph;
public:
    Any(): _ph(nullptr){};

    template<class T>
    Any(const T& everything) { _ph= new placeholder<T>(everything); }

    Any(const Any& other)
    {
        if(!other._ph)
        {
            std::cerr << "copy error" << std::endl;
            return;
        }

        _ph = other._ph->clone();
    }

    Any& swap(Any &other) 
    {
        std::swap(_ph, other._ph);
        return *this;
    }

    template<class T>
    Any& operator=(const T& everything)
    {
        Any tmp(everything);
        std::swap(_ph, tmp._ph);
        return *this;
    }

    Any& operator=(const Any &other)
    {
        Any tmp(other);
        std::swap(_ph, tmp._ph);
        return *this;
    }

    template<class T>
    T* Get_Value(){ return _ph ? &((placeholder<T>*)_ph)->_content : nullptr; }

    ~Any() { delete _ph; }
};

int main()
{
    // Any a(1);
    // Any b(2);
    // a = b;

    std::any a;
    a = std::string("hello world");
    std::cout << std::any_cast<std::string>(a) << std::endl;
    return 0;
}