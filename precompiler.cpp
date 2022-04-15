bool constexpr verbose = false;
bool constexpr print_all_scopes = true;
bool constexpr only_print_numbers = true;

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

#include <boost/algorithm/string/replace.hpp>  // replace_all

using std::size_t;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::to_string;
using std::vector;
using std::pair;

using std::istream_iterator;
using std::back_inserter;

string g_intact;

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

class CurlyBracketManager {
public:

    struct CurlyPair {
    protected:
    
        CurlyPair *_parent;

        pair<size_t,size_t> _indices;

        vector<CurlyPair> _nested;

    public:

        CurlyPair(size_t const first, CurlyPair *const arg_parent)
        {
            _indices.first  =      first;
            _indices.second =         -1;
                    _parent = arg_parent;
        }

        void clear(void) { _nested.clear(); }
        
        size_t First(void) const noexcept { return _indices.first ; }

        size_t Last (void) const noexcept { return _indices.second; }

        CurlyPair const *Parent(void) const;
        
        vector<CurlyPair> const &Nested(void) const noexcept { return _nested; }

        void SetLast(size_t const arg)
        {
            if ( -1 == _indices.first  ) throw std::runtime_error("SetLast should not be called on the root CurlyPair");
            if ( -1 != _indices.second ) throw std::runtime_error("This curly pair is being set for the second time!");
            if ( arg <= _indices.first ) throw std::runtime_error("Last char index must be > than first char index");

            _indices.second = arg;
        }
        
        CurlyPair *Add_New_Inner_Scope(size_t const first)
        {
            if ( (-1 != _indices.first) && (first <= _indices.first) ) throw std::runtime_error("Open bracket of inner scope must come after open bracket of outer scope");

            _nested.emplace_back(first, this);

            return &_nested.back();
        }
        
        CurlyPair *Close_Scope_And_Go_Back(size_t const last)
        {
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

        if constexpr ( false == only_print_numbers )
        {
            extern string Word_For_Curly_Pair(CurlyBracketManager::CurlyPair const &);
            str = Word_For_Curly_Pair(cp);
        }

        if ( false == str.empty() || print_all_scopes )
        {
            for ( size_t i = 0u; i != indentation; ++i )
            {
                cout << "    ";
            }

            cout << cp.First() << " (Line #" << LineOf(cp.First())+1u << "), " << cp.Last() << " (Line #" << LineOf(cp.Last())+1u << ")";


            if constexpr ( false == only_print_numbers )
            {
                extern string GetNames(CurlyBracketManager::CurlyPair const &);

                cout << "    " << GetNames(cp);
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

    bool Check_If_Root(CurlyPair const *) const;

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
    if ( nullptr == this->_parent ) throw std::runtime_error("Parent should never be invoked on the root pair");

    if ( &g_curly_manager._root_pair == _parent ) throw CurlyBracketManager::ParentError();

    return _parent;
}

string Word_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp)
{
    if ( cp.First() >= g_intact.size() || cp.Last() >= g_intact.size() )
    {
        throw std::runtime_error( string("Curly Pair is fucked [") + to_string(cp.First()) + "," + to_string(cp.Last()) + "]" );
    }

    size_t j = cp.First();

    if ( 0u == j ) return {};

    --j;  // j might become zero here

    while ( false == std::isspace(g_intact[j]) ) if ( 0u == j ) return {}; else --j;

    // control won't reach here if j is zero
    
    size_t i = j - 1u;  // i might become zero here

    while ( false == std::isspace(g_intact[i]) ) if ( 0u == i ) return {}; else --i;
    
    // control won't reach here if i is zero

    string const possible_class_name = g_intact.substr(i + 1u, j - i - 1u);  // REVISIT fix - check for rollover here

    //cout << "[" << possible_class_name << "] - - - ";

    j = i;

    if ( j == 0u || j == 1u ) return {};

    i = j - 1u;

    while ( false == std::isspace(g_intact[i]) ) if ( 0u == i ) break; else --i;
    
    string const clace = g_intact.substr(i + 1u, j - i - 1u);

    if      (     "class" == clace ) return possible_class_name;
    else if (    "struct" == clace ) return possible_class_name;
    else if ( "namespace" == clace ) return possible_class_name;

    return {};
}

string GetNames(CurlyBracketManager::CurlyPair const &cp)
{
    string retval = Word_For_Curly_Pair(cp);

    if ( retval.empty() ) return {};

    string tmp;

    size_t iteration = 0u;

    try
    {
        for ( CurlyBracketManager::CurlyPair const *p = &cp; p = p->Parent(); /* no post-processing */ )  // will throw exception when root pair is reached
        {
            assert( nullptr != p );

            verbose && cout << " / / / / / / About to call Word_For_Curly_Pair(" << p->First() << ", " << p->Last() << ")" << endl;
            tmp = Word_For_Curly_Pair(*p);
            verbose && cout << " / / / / / / Finished calling Word_For_Curly_Pair" << endl;

            if ( tmp.empty() ) continue;

            //retval.insert(0u, tmp + "::");
        }
    }
    catch(CurlyBracketManager::ParentError const &)
    {
        //cout << "********** ParentError ************";
    }

    return retval;
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
    g_curly_manager.Print();
}
