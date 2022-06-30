#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstdint>

using namespace std;

#define STR_LINKER " -> "
#define LEN_LINKER (sizeof(STR_LINKER)-1u)
#define LINK_SUFFIX " [style=solid];"

uintmax_t g_maxdepth = UINTMAX_MAX;

typedef vector<string> ChildContainer;
typedef map< string, ChildContainer > LinkContainer;

LinkContainer g_links;

inline void Print_Header(void) { cout << "digraph callgraph {" << endl; }
inline void Print_Footer(void) { cout << "}" << endl; }

/* The following function calls itself recursively. It is reentrant and thread-safe as it doesn't access any data of static-duration (nor any thread-local data). The C++ Standard functions called by this function e.g. "map::at, string::operator==" are hopefully implemented reentrant and thread-safe on your platform. */

static void Recursive_Is_Equal_Or_Is_Child_Of(string const &str_parent, string const &str_child, uintmax_t const level_of_recursion = 1)
{
        if ( str_parent == str_child )
                throw true;

        if ( level_of_recursion >= g_maxdepth )
                return;

        LinkContainer::mapped_type const *p_children;

        try        { p_children = &g_links.at(str_parent); } catch (...) { return; }  // The method 'at' will throw if no such key exists in the map

        for (LinkContainer::mapped_type::const_iterator p = p_children->begin(); p != p_children->end(); ++p)
        {
                if ( str_child == *p )
                        throw true;

                Recursive_Is_Equal_Or_Is_Child_Of(*p, str_child, level_of_recursion + 1);
        }
}

bool Is_Equal_Or_Is_Child_Of(string const &str_parent, string const &str_child)
{
        try
        {
                Recursive_Is_Equal_Or_Is_Child_Of(str_parent,str_child);
        }
        catch (bool)
        {
                return true;
        }

        return false;
}

void Print_With_New_Root(string const &str_root)
{
        Print_Header();

        LinkContainer::const_iterator p;

        for (p = g_links.begin(); p != g_links.end(); ++p)
        {
                LinkContainer::mapped_type::const_iterator q;

                for (q = p->second.begin(); q != p->second.end(); ++q)
                {
                        if ( Is_Equal_Or_Is_Child_Of(str_root, p->first) )
                        {
                                cout << "\"" << p->first << "\" -> \"" << *q << "\"" << LINK_SUFFIX << endl;
                        }
                }
        }        

        Print_Footer();
}

int main(int argc, char **argv)
{
        // USAGE: DOT_New_Root 3 main
        // This sets 'main' as the root and goes down to a maximum depth of 3
        // (For unlimited depth, specify 0)

        if (3 != argc)
                return EXIT_FAILURE;

        stringstream( string(argv[1]) ) >> g_maxdepth;

        if ( 0 == g_maxdepth )
                g_maxdepth = UINTMAX_MAX;

        string line;

        while ( getline(cin,line) )
        {
                size_t pos = line.find(STR_LINKER);  // Look for the linkage symbol "  ->  "

                if ( string::npos == pos )
                        continue;

                string str_parent = line.substr(1, pos-2);

                string str_child  = line.substr(pos + LEN_LINKER + 1, string::npos );

                str_child.resize( str_child.find_first_of('\"') );

                g_links[str_parent].push_back(str_child);
        }

        Print_With_New_Root(argv[2]);

        return 0;
}
