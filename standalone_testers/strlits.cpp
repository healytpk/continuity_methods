#include <unordered_map>
#include <string>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;

string g_intact;

bool constexpr verbose = true;

void Replace_All_String_Literals_With_Spaces(bool undo = false)
{
    static std::unordered_map<size_t, string> strlits;

    if ( undo )
    {
        for ( auto const &e : strlits )
        {
            std::memcpy(&g_intact[e.first], &e.second.front(), e.second.size());
        }

        return;
    }

    size_t count = 0u;

    for ( size_t i = 0u; i != g_intact.size(); ++i )
    {
        assert( '\r' != g_intact[i] );
        //assert( '\t' != g_intact[i] );

        if ( '"' == g_intact[i] )
        {
            if ( (0u != i) && ('\\' == g_intact[i-1u]) ) throw runtime_error("Found a stray escaped double-quote in translation uint");

            ++i;  // Now it's the index of the first char in the string literal

            ++count;
            
            assert( 1u == (count % 2u) );

            if ( g_intact.size() == (i + 1u) )
            {
                // The double-quote symbol is the last char in the file
                throw runtime_error("The last char in the file is an unmatched double-quote -- bailing out");
            }

            for ( size_t j = i; /* ever */ ;)
            {
                // We have a string literal starting at index i
                size_t k = g_intact.find_first_of('"', j);

                if ( -1 == k ) throw runtime_error("Unmatched double-quote in translation unit");

                if ( '\\' == g_intact[k - 1u] )  // ignored double-quotes that are escaped, e.g. "My dog is a \"pure breed\" dog."
                {
                    j = k + 1u;
                    continue;
                }

                ++count;

                assert( 0u == (count % 2u) );

                if ( k == i )
                {
                    break;  // if we have for example "std::puts("");"
                }

                strlits[i] = g_intact.substr(i, k - i);

                clog << "Replacing string literal at index " << i << " : [" << strlits[i] << "]" << endl;

                std::memset(&g_intact[i], ' ', k - i);

                i = k;  // i will be incremented on the next 'for' iterator
                break;
            }

            if ( 1u == (count % 2u) ) throw runtime_error("Unmatched double-quote in translation unit");
        }
    }

    clog << "String Literal Replacements:\n"
            "============================\n";

    for ( auto const &e : strlits )
    {
        clog << "Index " << e.first << ", Len = " << e.second.size() << ", [" << e.second << "]" << endl;
    }
}

int main(void)
{
    cin >> std::noskipws;

    g_intact.reserve(5000u);

    std::copy( istream_iterator<char>(cin), istream_iterator<char>(), back_inserter(g_intact) );

    //cout << g_intact << endl;

    Replace_All_String_Literals_With_Spaces();

    //cout << g_intact << endl;

    Replace_All_String_Literals_With_Spaces(true);

    cout << g_intact;
}
