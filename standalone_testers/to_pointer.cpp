#include <type_traits>  // enable_if, is_pointer
#include <utility>      // declval

template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
T to_address(T arg)  // Pass pointers by value
{
    return arg;
}

template <typename T,
          typename std::enable_if<false == std::is_pointer<T>::value, bool>::type = false,
          typename std::enable_if<std::is_pointer< decltype(std::declval<T>().operator->()) >::value, bool>::type = true>
decltype(std::declval<T>().operator->()) to_address(T arg)  // Pass iterators by value
{
    return arg.operator->();
}

#include <string>
using std::string;

int main(void)
{
    char monkey[64];

    char const volatile *const volatile p = monkey;

    string frog("frog");

    string const donkey("donkey");

    to_address( frog.end() );
    to_address( frog.cend() );
    
    to_address( donkey.end() );
    
    to_address( monkey );

    // Let's try reverse iterators too:
    to_address( frog.rbegin()  );
    to_address( donkey.crend() );
}
