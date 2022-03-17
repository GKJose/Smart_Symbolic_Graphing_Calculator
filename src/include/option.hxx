#ifndef OPTION_HEADER
#define OPTION_HEADER
#include <cstdlib>


#define _OPTION_DEF2(name, op) template<typename R> \
			auto name (R val){ \
				return this op val; \
			}

#define _OPTION_DEF1(op) template<typename R>\
			auto operator op(R const& val){\
				if (!empty)\
					return Option<decltype(value op _value)>(_value op value);\
				return Option<decltype(value op _value)>::nothing();\
				}

struct None{};

static None OptNone;

template<typename T>
class Option{
	T _value;
    bool empty;
public:
    using Self = Option<T>;


    Option():empty(true){}
    explicit Option(T value):empty(false),_value(std::move(value)){}
    Option(None):empty(true){}

    constexpr bool is_empty() const noexcept{
        return empty;
    }
    constexpr T value() const noexcept{
        return _value;
    }
    constexpr T& value_ref(){
        return _value;
    }

    template<typename R>
	Option<R> map(R (*func)(T)){
        using NewSelf = Option<R>;
        if (!empty){
            return NewSelf(func(_value));
        }
        return NewSelf::none();
    }
    void apply(T (*func)(T)){
        if (!empty)
		    _value = func(value);
    }
    static Self none() noexcept{
        return Self();
    }

    _OPTION_DEF1(+)
    _OPTION_DEF1(-)
    _OPTION_DEF1(*)
    _OPTION_DEF1(/)
    _OPTION_DEF1(==)

    _OPTION_DEF2(add, +)
    _OPTION_DEF2(sub, -)
    _OPTION_DEF2(mul, *)
    _OPTION_DEF2(div, /)
    _OPTION_DEF2(equals, ==)

    void operator=(T value){
        _value = std::move(value); empty = false;
    }
    operator bool() const{
        return !empty;
    }
    
};

#endif