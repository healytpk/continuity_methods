#include <cstddef>      // size_t
#include <string>
#include <string_view>
#include <iostream>
#include <cctype>       // isspace
#include <regex>

using std::string;
using std::string_view;
using std::cout;
using std::endl;

namespace psuedoboost {

    void erase_all(string &s, string_view const sv)
    {
        size_t i = 0;
        while ( (s.size() != i) && (-1 != (i = s.find(sv, i))) )  // deliberate single '='
        {
            s.erase(i, sv.size());
        }
    }

    void replace_all(string &s, string_view const sv_old, string_view const sv_new)
    {
        size_t i = 0;
        while ( (s.size() != i) && (-1 != (i = s.find(sv_old, i))) )  // deliberate single '='
        {
            s.erase(i, sv_old.size());
            s.insert(i, sv_new);
            i += sv_new.size();
        }
    }

    void trim_all(string &s)
    {
        if ( s.empty() ) return;

        for ( auto &e : s )
        {
            if ( std::isspace(e) )
            {
                e = ' ';
            }
        }

        for( size_t i = s.size() - 1u; 0 != i; --i )
        {
            if ( ' ' == s[i] && ' ' == s[i-1u] )  // REVISIT FIX - make more efficient
            {
                s.erase(i, 1u);
            }
        }
    }
}

int main(void)
{
    string str("The dog                 swam quickly across the nicely-decorated lake");

    using namespace psuedoboost;

    erase_all(str,"ly");
    
    replace_all(str, "across", "beneath the surface of" );

    trim_all(str);
    
    cout << str << endl;
}
