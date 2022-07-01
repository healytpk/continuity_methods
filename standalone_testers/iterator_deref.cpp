#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

using namespace std;

#if 0

void Increment_First_And_Print_Without_First(string &s)
{
    if ( s.size() < 2u ) throw -1;

    ++( s[0u] );

    // The next line has an invalid dereferencing of an iterator
    cout << "The next line does NOT dereference an end() pointer" << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() - 1u) + 1u ) << endl;

    cout << "The next line dereferences an end() pointer" << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend()) ) << endl;
}

int main(void)
{
    cout << "string::const_iterator is "
         << (is_same_v< string::const_iterator, char const * > ? "just a raw pointer" : "NOT a simple pointer") << endl;

    cout << "string_view::const_iterator is "
         << (is_same_v< string_view::const_iterator, char const * > ? "just a raw pointer" : "NOT a simple pointer") << endl;

    string str("brush");

    Increment_First_And_Print_Without_First(str);

    cout << str << endl;
}

#else

int main(void)
{
    cout << "string::const_iterator is "
         << (is_same_v< string::const_iterator, char const * > ? "just a raw pointer" : "NOT a simple pointer") << endl;

    cout << "string_view::const_iterator is "
         << (is_same_v< string_view::const_iterator, char const * > ? "just a raw pointer" : "NOT a simple pointer") << endl;

    string s("brush");

    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 1u) ) << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 2u) ) << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 3u) ) << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 4u) ) << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 5u) ) << endl;
    cout << string_view( &*(s.cbegin() + 1u), &*(s.cend() + 6u) ) << endl;
}

#endif
