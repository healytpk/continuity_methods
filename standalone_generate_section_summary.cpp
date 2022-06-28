#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <string_view>
using namespace std;

vector< pair<unsigned, string> > records;

int main(int const argc, char **const argv)
{
    std::ifstream file("precompiler.cpp");

	unsigned i = 0u;
    unsigned total_count_lines = 0u;
	string line;

	while ( std::getline(file, line) )
    {
        ++i;

        ++total_count_lines;

        if ( string_view(line).contains("Section") && (false == string_view(line).starts_with(" * ")) )
        {
            records.push_back( pair<unsigned,string>(i - 1u, line) );
        }
    }

    for ( unsigned i = 0; i != records.size(); ++i )
    {
        pair<unsigned,string> &e = records[i];

        string const &line = e.second;

        unsigned index = 0u;

        for ( unsigned j = 0u; j != 6; ++j )
        {
            index = line.find(' ', index) + 1u;
        }

        cout << " * (Section ";

        if ( i+1u < 10u ) cout << ' ';

        cout << i+1u << ") ";

        cout << line.substr(index);

        for ( unsigned i = 0u; i < (57u - line.substr(index).size()); ++i ) cout << " ";

        cout << " : Line #";

        if ( e.first < 1000u ) cout << ' ';        
        if ( e.first <  100u ) cout << ' ';
        if ( e.first <   10u ) cout << ' ';

        unsigned a = e.first;
        unsigned b;

        cout << e.first << " - #";

        if ( (i + 1u) < records.size() )
        {
            pair<unsigned,string> &e = records[i + 1u];

            b = e.first-2u;

            if ( e.first-2u < 1000u ) cout << ' ';        
            if ( e.first-2u <  100u ) cout << ' ';
            if ( e.first-2u <   10u ) cout << ' ';

            cout << e.first-1u;
        }
        else
        {
            b = total_count_lines;

            if ( total_count_lines < 1000u ) cout << ' ';        
            if ( total_count_lines <  100u ) cout << ' ';
            if ( total_count_lines <   10u ) cout << ' ';

            cout << total_count_lines;
        }

        string blah = std::to_string(b-a + 2u);

        cout << " : (Total = ";

        if ( blah.size() < 4u ) cout << ' ';
        if ( blah.size() < 3u ) cout << ' ';
        if ( blah.size() < 2u ) cout << ' ';

        cout << blah << " lines)" << endl;
    }

    return 0;

	i = 0u;

	while ( std::getline(cin, line) )
    {
        ++i;

        cout << " * (Section ";

        if ( i < 10u ) cout << " ";

        cout << i << ") " ;
    }
}
