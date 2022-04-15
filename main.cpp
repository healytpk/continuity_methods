#include <cstddef>    // size_t
#include <cstdlib>    // EXIT_FAILURE
#include <iostream>   // cout, endl
#include <fstream>    // ifstream
#include <algorithm>  // copy, replace, count
#include <iterator>   // next, back_inserter, istream_iterator
#include <string>     // string
#include <ios>        // ios::binary
#include <iomanip>    // noskipws
#include <stdexcept>  // out_of_range
#include <utility>    // pair<>

#include <boost/algorithm/string/replace.hpp>  // replace_all

using std::size_t;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;
using std::pair;

using std::istream_iterator;
using std::back_inserter;

string g_intact;

inline void ThrowIfBadIndex(size_t const char_index)
{
    if ( char_index >= g_intact.size() )
        throw std::out_of_range("Cannot access *(p + " + std::to_string(char_index) + ") inside type char[" + std::to_string(g_intact.size()) + "]");    
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

        pair<size_t,size_t> _indices;
        vector<CurlyPair> _nested;

        CurlyPair *_parent;

        CurlyPair(size_t const first, size_t const last, CurlyPair *const arg_parent)
        {
            _indices.first  = first     ;
            _indices.second = last      ;
                    _parent = arg_parent;
        }
    };

    vector<CurlyPair> _pairs;

    void Process(void)
    {
        size_t i = g_intact.find_first_of('{');

        if ( -1 == i ) throw std::runtime_error("Input file doesn't contain an open curly bracket");

        _pairs.clear();

        _pairs.emplace_back(0u, 0u, nullptr);

        _pairs.front()._nested.emplace_back(i, -1, &_pairs.front());

        CurlyPair *current = & (_pairs.front()._nested.front());

        for ( ++i; i != g_intact.size(); ++i )
        {
            char const c = g_intact[i];

            if ( '{' == c )
            {
                current->_nested.emplace_back(i, -1, current);
                current = &current->_nested.back();
            }
            else if ( '}' == c )
            {
                current->_indices.second = i;
                current = current->_parent;
            }
        }
    }
    
    void Print_CurlyPair(CurlyPair const &cp, size_t const indentation = 0u) const
    {
        if ( nullptr != cp._parent )
        {
            for ( size_t i = 0; i != indentation; ++i )
            {
                cout << "    ";
            }

            extern string GetNames(CurlyBracketManager::CurlyPair const *);
            
            cout << cp._indices.first << " (Line #" << LineOf(cp._indices.first)+1u << "), " << cp._indices.second << " (Line #" << LineOf(cp._indices.second)+1u << ")";
            
            cout << "    " << GetNames(&cp);
            
            cout << endl;
        }

        for (CurlyPair const e : cp._nested)
        {
            Print_CurlyPair(e, indentation + 1u);
        }
    }
    
    void Print(void) const
    {
        Print_CurlyPair( _pairs.front() );
    }

} g_curly_manager;

string Word_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp)
{
    size_t j = cp._indices.first;
    
    --j;  // REVISIT FIX - Might be zero
    
    while ( ' ' != g_intact[j] ) if ( 0u == j ) return {}; else --j;
    
    size_t i = j - 1u;
    
    while ( ' ' != g_intact[i] ) if ( 0u == i ) return {}; else --i;
    
    string possible_class_name = g_intact.substr(i + 1u, j - i - 1u);
    
    //cout << "[" << possible_class_name << "] - - - ";
    
    j = i;

    if ( j == 1u ) return {};

    i = j - 1u;

    while ( ' ' != g_intact[i] && '\n' != g_intact[i] && '\t' != g_intact[i] ) if ( 0u == i ) break; else --i;
    
    string clace = g_intact.substr(i + 1u, j - i - 1u);

    if      (     "class" == clace ) return possible_class_name;
    else if (    "struct" == clace ) return possible_class_name;
    else if ( "namespace" == clace ) return possible_class_name;
    
    return {};
}

string GetNames(CurlyBracketManager::CurlyPair const *p)
{
    if ( nullptr == p ) return {};

    string retval = Word_For_Curly_Pair(*p);

    if ( retval.empty() ) return {};

    while ( p = p->_parent )
    {
        if ( p == &g_curly_manager._pairs.front() ) break;

        //if ( nullptr == p->_parent ) break;

        string tmp = Word_For_Curly_Pair(*p);

        if ( tmp.empty() ) continue;

        retval = tmp + "::" + retval;
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
