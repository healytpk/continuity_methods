#include <cstdlib>    // EXIT_FAILURE
#include <iostream>   // cin, cout
#include <fstream>    // ifstream
#include <ios>        // noskipws

using std::cin;
using std::cout;

int main(int const argc, char **const argv)
{
    if ( 2u != argc ) return EXIT_FAILURE;

    std::ifstream f(argv[1u]);

    if ( false == f.is_open() )
    {
        cout << "Cannot open file: " << argv[1u] << "\n";
        return EXIT_FAILURE;
    }

    cin >> std::noskipws;

    char bufA[2u] = { 'x', '\0' },  // null-terminated
         bufB[2u] = { 'x', '\0' };

    while ( f.read(bufA, 1u) )
    {
        if ( '\r' == bufA[0u] ) continue;

        bool satisfied = false;

        while ( cin.read(bufB, 1u) )
        {
            if ( '\r' == bufB[0u] ) continue;

            if ( bufA[0u] != bufB[0u] )
            {
                cout << "Mismatch between '" << bufA[0u] << "' and '" << bufB[0u] << "'\n";
                return EXIT_FAILURE;
            }

            satisfied = true;

            break;
        }

        if ( false == satisfied ) return EXIT_FAILURE;
    }

    while ( cin.read(bufB, 1u) )
    {
        if ( '\r' == bufB[0u] ) continue;

        return EXIT_FAILURE;
    }

    return 0;
}
