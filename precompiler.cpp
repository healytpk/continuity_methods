bool constexpr verbose = false;
bool constexpr print_all_scopes = false;

bool only_print_numbers; /* This gets set in main -- don't set it here */

#include <cstddef>    // size_t
#include <cassert>    // assert
#include <cstdlib>    // EXIT_FAILURE
#include <iostream>   // cout, endl
#include <fstream>    // ifstream
#include <algorithm>  // copy, replace, count
#include <iterator>   // next, back_inserter, istream_iterator
#include <string>     // string, to_string
#include <ios>        // ios::binary
#include <iomanip>    // noskipws
#include <stdexcept>  // out_of_range, runtime_error
#include <utility>    // pair<>
#include <cctype>     // isspace
#include <list>       // list
#include <map>        // map
#include <ranges>     // views::filter
#include <tuple>      // tuple
#include <utility>    // pair
#include <vector>     // vector
#include <string_view> // string_view

#include <boost/algorithm/string/trim_all.hpp>  // trim_all
#include <boost/algorithm/string/replace.hpp>   // replace_all
#include <boost/algorithm/string/erase.hpp>     // erase_all

using std::size_t;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::to_string;
using std::string_view;
using std::vector;
using std::tuple;
using std::pair;

using std::istream_iterator;
using std::back_inserter;
using std::views::filter;
using std::views::split;

string g_intact;

std::map< string, pair< string, vector< tuple<string,string,string> > > > g_scope_names;

/* For example:

get<0>    class
get<1>    Laser_NitrogenPicoSecond
get<2>    {
              { "" "public" "Laser_Nitrogen"   }
              { "" "public" "Laser_PicoSecond" }
          }
*/

inline void ThrowIfBadIndex(size_t const char_index)
{
    if ( char_index >= g_intact.size() )
        throw std::runtime_error("Cannot access *(p + " + std::to_string(char_index) + ") inside type char[" + std::to_string(g_intact.size()) + "]");    
}

inline size_t LastChar(void) { return g_intact.size() - 1u; }

inline size_t Lines(void) { return std::count( g_intact.begin(), g_intact.end(), '\n' ); }

inline size_t LineOf(size_t const char_index)
{
    ThrowIfBadIndex(char_index);

    return std::count( g_intact.begin(), std::next(g_intact.begin(), char_index), '\n' );
}

inline size_t StartLine(size_t char_index)
{
    ThrowIfBadIndex(char_index);

    while ( (0u != char_index) && ('\n' != g_intact[char_index]) )
    {
        --char_index;
    }
    
    return char_index;
}

inline size_t EndLine(size_t char_index)
{
    ThrowIfBadIndex(char_index);

    while ( (LastChar() != char_index) && ('\n' != g_intact[char_index]) )
    {
        ++char_index;
    }
    
    return char_index;
}

string TextBeforeOpenCurlyBracket(size_t const char_index)
{
    ThrowIfBadIndex(char_index);

    if ( 0u == char_index ) return {};

    if ( '{' != g_intact[char_index] ) throw std::runtime_error("This isn't an open curly bracket!");

    size_t i = char_index;

    while ( --i )
    {
        bool break_out_of_loop = false;

        switch ( g_intact[i] )
        {
        case '}':
        case '{':
        case ';':
        case '(':
        case ')':
        case '<':
        case '>':

            break_out_of_loop = true;
            break;

        default:

            continue;
        }

        if ( break_out_of_loop ) break;
    }

    string retval = g_intact.substr(i + 1u, char_index - i - 1u);   // REVISIT FIX might overlap

    boost::algorithm::trim_all(retval);

    return retval;
}

class CurlyBracketManager {
public:

    struct CurlyPair {
    protected:

        CurlyPair *_parent;
        pair<size_t,size_t> _indices;
        std::list<CurlyPair> _nested;

    public:

        CurlyPair(size_t const first, CurlyPair *const arg_parent)
        {
            _indices.first  =      first;
            _indices.second =         -1;
                    _parent = arg_parent;

            //_nested.reserve(64u);  // We get memory corruption without this - this only applied to vector when it resized, and relocated objects (invalidating iterators)
        }

        void clear(void) { _nested.clear(); }
        
        size_t First(void) const noexcept { return _indices.first ; }

        size_t Last (void) const noexcept { return _indices.second; }

        CurlyPair const *Parent(void) const;
        
        std::list<CurlyPair> const &Nested(void) const noexcept { return _nested; }

        CurlyPair *Add_New_Inner_Scope(size_t const first)
        {
            if ( nullptr == this ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( (-1 != _indices.first) && (first <= _indices.first) ) throw std::runtime_error("Open bracket of inner scope must come after open bracket of outer scope");

            _nested.emplace_back(first, this);

            return &_nested.back();
        }
        
        CurlyPair *Close_Scope_And_Go_Back(size_t const last)
        {
            if ( nullptr == this ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( nullptr ==        _parent ) throw std::runtime_error("The root pair is NEVER supposed to get closed");
            if (    last <= _indices.first ) throw std::runtime_error("Closing backet of inner scope must come after open bracket");

            _indices.second = last;

            return _parent;
        }
    };

protected:

    CurlyPair _root_pair{ (size_t)-1, nullptr};

    void Print_CurlyPair(CurlyPair const &cp, size_t const indentation = 0u) const
    {
        verbose && cout << "- - - - - Print_CurlyPair( *(" << &cp << "), " << indentation << ") - - - - -" << endl;

        string str;

        extern tuple< string,string, vector< tuple<string,string,string> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp);

        if ( false == only_print_numbers )
        {
            str = std::get<1u>(Intro_For_Curly_Pair(cp));
        }

        if ( false == str.empty() || print_all_scopes )
        {
            for ( size_t i = 0u; i != indentation; ++i )
            {
                cout << "    ";
            }

            cout << cp.First() << " (Line #" << LineOf(cp.First())+1u << "), " << cp.Last() << " (Line #" << LineOf(cp.Last())+1u << ")";

            verbose && cout << "  [Full line: " << TextBeforeOpenCurlyBracket(cp.First()) << "]";

            if ( false == only_print_numbers )
            {
                extern string GetNames(CurlyBracketManager::CurlyPair const &);

                //cout << "    " << GetNames(cp);

                cout << "  [" << GetNames(cp) << "]";
            }

            cout << endl;
        }

        for ( CurlyPair const &e : cp.Nested() )
        {
            verbose && cout << "    - - - About to recurse" << endl;
            Print_CurlyPair(e, indentation + 1u);
        }
    }

public:

    struct ParentError : std::exception { };

    void Process(void)
    {
        verbose && cout << "========= STARTING PROCESSING ===========" << endl;

        _root_pair.clear();

        CurlyPair *current = &_root_pair;

        for ( size_t i = 0u; i != g_intact.size(); ++i )
        {
            char const c = g_intact[i];

            if ( '{' == c )
            {
                current = current->Add_New_Inner_Scope(i);
            }
            else if ( '}' == c )
            {
                current = current->Close_Scope_And_Go_Back(i);
            }
        }

        verbose && cout << "========= ENDING PROCESSING ===========" << endl;
    }
    
    
    void Print(void) const
    {
        for ( CurlyPair const &e : _root_pair.Nested() )
        {
            verbose && cout << "    - - - About to recurse" << endl;
            Print_CurlyPair(e);
        }
    }

} g_curly_manager;

CurlyBracketManager::CurlyPair const *CurlyBracketManager::CurlyPair::Parent(void) const
{
    if ( nullptr == this          ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");
    if ( nullptr == this->_parent ) throw std::runtime_error("Parent() should never be invoked on the root pair");

    if ( &g_curly_manager._root_pair == this->_parent ) throw CurlyBracketManager::ParentError();

    return this->_parent;
}

vector< tuple<string,string,string> > Parse_Bases_Of_Class(string_view const str)
{
    tuple<string,string,string> tmp;

    vector< tuple<string,string,string> > retval;

    for ( auto const &base : str | split(',') )
    {
        for ( auto const &word_view : base |  split(' ') )
        {
            string word;

            for ( char const c : word_view )
            {
                word += c;
            }

            boost::trim_all(word);

            if ( "virtual" == word )
            {
                std::get<0u>(tmp) = "virtual";
            }
            else if ( "public" == word || "protected" == word || "private" == word )
            {
                std::get<1u>(tmp) = word;
            }
            else
            {
                std::get<2u>(tmp) = word;  // This is the name of the base class
            }
        }

        //cout << "[ADD RECORD : " << std::get<0u>(tmp) << " : " << std::get<1u>(tmp) << " : " << std::get<2u>(tmp) << "]";
        retval.push_back(tmp);
    }

    return retval;
}

tuple< string, string, vector< tuple<string,string,string> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp)
{
    if ( cp.First() >= g_intact.size() || cp.Last() >= g_intact.size() )
    {
        throw std::runtime_error( string("Curly Pair is corrupt [") + to_string(cp.First()) + "," + to_string(cp.Last()) + "]" );
    }

    string intro = TextBeforeOpenCurlyBracket(cp.First());

    if ( intro.starts_with("namespace") )
    {
        string str;

        unsigned i = 0u;
        for ( auto word : intro | std::views::split(' ') )
        {
            if ( 1u != i++ ) continue;

            for (char ch : word)
            {
                str += ch;
            }

            break;
        }

        return { "namespace", str, {} };
    }

    if (   !(intro.starts_with("class") || intro.starts_with("struct"))   ) return {};

    boost::erase_all( intro, " final" );   // careful it might be "final{"

    auto my_view = intro | std::views::split(' ');

    string str;

    unsigned i = 0u;
    auto pword = my_view.begin();
    for ( ; pword != my_view.end(); ++pword)
    {
        auto word = *pword;

        if ( 1u != i++ ) continue;

        for (char ch : word)
        {
            str += ch;
        }

        break;
    }

    if ( ++pword == my_view.end() ) return { "class", str, {} };
    if ( ++pword == my_view.end() ) return { "class", str, {} };

    return { "class", str, Parse_Bases_Of_Class( string_view( &(*pword).front(), &intro.back() + 1u ) ) };
}

string GetNames(CurlyBracketManager::CurlyPair const &cp)
{
    tuple< string, string, vector< tuple<string,string,string> >  > tmppair = Intro_For_Curly_Pair(cp);

    string retval = std::get<1u>(tmppair);  /* e.g. 0 = class, 1 = Laser, 2 = [ ... base classes ... ] */

    if ( retval.empty() ) return {};

    size_t iteration = 0u;

    try
    {
        for ( CurlyBracketManager::CurlyPair const *p = &cp; p = p->Parent(); /* no post-processing */ )  // will throw exception when root pair is reached
        {
            //cout << "Iteration No. " << iteration << endl;

            verbose && cout << " / / / / / / About to call Word_For_Curly_Pair(" << p->First() << ", " << p->Last() << ")" << endl;
            string const tmp = std::get<1u>( Intro_For_Curly_Pair(*p) );
            verbose && cout << " / / / / / / Finished calling Word_For_Curly_Pair" << endl;

            if ( tmp.empty() ) continue;

            retval.insert(0u, tmp + "::");
        }
    }
    catch(CurlyBracketManager::ParentError const &)
    {
        //cout << "********** ParentError ************";
    }

    retval.insert(0u, "::");

    g_scope_names[retval] = { std::get<0u>(tmppair), std::get<2u>(tmppair) };

    return retval;
}

void Recursive_Print_All_Bases(string const &arg)
{

    decltype(g_scope_names)::mapped_type const *p = nullptr;

    try
    {
        p = &( g_scope_names.at(arg) );
    }
    catch (std::out_of_range const &)
    {
        return;
    }

    assert( nullptr != p );

    for ( auto const &e : p->second )
    {
        string const &base_name = std::get<2u>(e);

        cout << base_name << ", ";

        Recursive_Print_All_Bases(arg.substr(0u, arg.rfind("::") + 2u) + base_name);
    }
}

int main(int const argc, char **const argv)
{
    std::ifstream f(argv[1u], std::ios::binary);

    f >> std::noskipws;

    if ( false == f.is_open() ) { cout << "Cannot open file" << endl; return EXIT_FAILURE; }

    std::copy( istream_iterator<char>(f), istream_iterator<char>(), back_inserter(g_intact) );

    cout << "Bytes: " << g_intact.size() << endl;
    cout << "Lines: " << Lines() << endl;

    std::replace(g_intact.begin(), g_intact.end(), '\0', ' ');

    boost::replace_all(g_intact, "\r\n", " \n");

    boost::replace_all(g_intact, "\r", "\n");

    //std::copy( g_intact.begin(), g_intact.end(), std::ostream_iterator<char>(cout) );
    
    g_curly_manager.Process();

    cout << "====================================== ALL PROCESSING DONE ===========================================" << endl;

    only_print_numbers = false;
    g_curly_manager.Print();

    cout << "====================================== All scope names ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        cout << e.first << " [" << e.second.first << "]" << endl;
    }

    cout << "====================================== Now the namespaces ==============================================" << endl;
    for ( auto const &e : g_scope_names | filter([](auto const &arg){ return "namespace" == arg.second.first; }) )
    {
        cout << e.first << endl;
    }

    cout << "====================================== Now the classes ==============================================" << endl;
    for ( auto const &e : g_scope_names | filter([](auto const &arg){ return "class" == arg.second.first || "struct" == arg.second.first; }) )
    {
        cout << e.first << " | Bases = ";

        Recursive_Print_All_Bases(e.first);

        cout << endl;
    }
}
