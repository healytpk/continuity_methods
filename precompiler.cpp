/*
 * This program is one file of C++ code, and has no dependencies other
 * than the C++ 2020 standard library, and Boost 1.76.
 *
 * The purpose of this program is to implement the C++ proposal entitled
 * 'Continuity Methods'.
*/

// =================================================================
// Section 1 of 9 : Override global 'new' and 'delete' for max speed
// =================================================================

decltype(sizeof(1)) g_total_allocation = 0u;

#if 0  // Change this line to "#if 0" in order to disable this feature

#include <cstddef>    // size_t
#include <cstdlib>    // malloc
#include <new>        // required otherwise we get a compiler error for the 'noexcept'

using std::size_t;

inline void *Implementation_Global_New(size_t size) noexcept
{
    size += 8u - (size % 8u);

    static size_t constexpr bytes_at_a_time = 10485760u;  // 10 megabytes

    if ( size > bytes_at_a_time ) return nullptr;

    static void *p = nullptr;

    static size_t bytes_allocated_so_far = 0u;

    for (; /* ever */ ;)
    {
        if ( nullptr == p )
        {
            p = std::malloc(bytes_at_a_time);

            if ( nullptr == p ) return nullptr;

            g_total_allocation += bytes_at_a_time;
        }

        if ( (bytes_allocated_so_far + size) > bytes_at_a_time )
        {
            p = nullptr;

            bytes_allocated_so_far = 0u;
        }
        else
        {
            break;
        }
    }

    void *const retval = static_cast<char*>(p) + bytes_allocated_so_far;

    bytes_allocated_so_far += size;

    return retval;
}

void *operator new  (size_t const size) noexcept { return Implementation_Global_New(size); }
void *operator new[](size_t const size) noexcept { return Implementation_Global_New(size); }
void operator delete  (void *const p) noexcept { /* Do Nothing */ }
void operator delete[](void *const p) noexcept { /* Do Nothing */ }

#endif // if override global 'new' and 'delete' for max speed is enabled

// ==========================================================================
// Section 2 of 9 : Implementations of functions from Boost
// ==========================================================================

#include <cassert>     // assert
#include <cstddef>     // size_t
#include <cstring>     // strcmp, strlen
#include <cctype>      // isalnum
#include <string>      // string
#include <string_view> // string_view
#include <type_traits> // remove_reference_t, remove_cv_t, decay (all for C++17)

class StringAlgorithms {

private:

    static void trim_all(std::string &s)
    {
        if ( s.empty() ) return;

        std::size_t i;
        for( i = s.size() - 1u; 0u != i; --i )
        {
            if ( std::isspace(static_cast<char unsigned>(s[i])) )  // Replace any whitespace character with a space
            {
                s[i] = ' ';

                if (std::isspace(static_cast<char unsigned>(s[i - 1u])) )  // Erase space if there's a whitespace character before it
                {
                    s.erase(i, 1u);
                }
            }
        }

        assert( 0u == i );

        if ( std::isspace(static_cast<char unsigned>(s.front())) )
        {
            s.erase(0,1u);
        }
    }

public:

    static void erase_all(std::string &s, std::string_view const sv)
    {
        if ( s.empty() || sv.empty() ) return;

        std::size_t i = 0u;
        while ( (s.size() != i) && (-1 != (i = s.find(sv, i))) )  // deliberate single '='
        {
            s.erase(i, sv.size());
        }
    }

    static void replace_all(std::string &s, std::string_view const sv_old, std::string_view const sv_new)
    {
        if ( s.empty() || sv_old.empty() ) return;

        std::size_t i = 0u;
        while ( (s.size() != i) && (-1 != (i = s.find(sv_old, i))) )  // deliberate single '='
        {
            s.erase(i, sv_old.size());

            if ( false == sv_new.empty() )
            {
                s.insert(i, sv_new);
                i += sv_new.size();
            }
        }
    }

private:

    static bool IsIdChar(char const c)
    {
        return '_' == c || std::isalnum(static_cast<char unsigned>(c));
    }

    static bool Is_Space_Necessary(std::string_view const s, std::size_t const i)
    {
        // Input string has already had all white
        // space reduced to one space, and it
        // neither ends nor begins with a space

        static char const *const double_syms[] = { "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "||", "&&", "==", "!=", ">=", "<=", "<<", ">>", ".*", "++", "--", "->", "::" };

        static char const *const tripple_syms[] = { "<<=", ">>=", "<=>", "->*" };

        assert( ' ' == s[i] );

        if ( IsIdChar(s[i-1u]) && IsIdChar(s[i+1u]) )
        {
            return true;
        }

        for ( char const *const sss : tripple_syms )
        {
            char monkey[4u] = {};

            monkey[0u] = s[i - 1u];
            monkey[1u] = s[i + 1u];
            monkey[2u] = s[i + 2u];

            if ( 0 == std::strcmp(sss,monkey) ) return true;

            if ( 1u == i ) continue;

            monkey[0u] = s[i - 2u];
            monkey[1u] = s[i - 1u];
            monkey[2u] = s[i + 1u];

            if ( 0 == std::strcmp(sss,monkey) ) return true;
        }

        for ( char const *const ss : double_syms )
        {
            char monkey[3u] = {};

            monkey[0u] = s[i - 1u];
            monkey[1u] = s[i + 1u];

            if ( 0 == std::strcmp(ss,monkey) ) return true;
        }

        return false;
    }

    static void Remove_Unnecessary_Spaces(std::string &s)
    {
        for ( std::size_t i = 0u;
                  i != s.size()
              && -1 != (i = s.find(' ',i));
              ++i )
        {
            if ( Is_Space_Necessary(s, i) ) continue;

            s.erase(i--,1u);
        }
    }

public:

    static void Minimise_Whitespace(std::string &s)
    {
        trim_all(s);  // Removes leading whitespace, trailing whitespace, and reduces all other whitespace to one space ' '
        Remove_Unnecessary_Spaces(s);
    }
};

// The following function is to support C++17 which is lacking the
// constructor for 'string_view' which takes two iterators. Also we want
// to be able to mix and match different iterator types.
template<typename A, typename B>
static std::string_view Sv(A a, B b)  // Pass iterators by value
{
    using std::is_same_v;
    using std::string;
    using std::string_view;

    typedef std::remove_cv_t< std::remove_reference_t<A> > X;  // C++17 doesn't have remove_cvref_t
    typedef std::remove_cv_t< std::remove_reference_t<B> > Y;

    static_assert(    is_same_v< X, char *                      >
                   || is_same_v< X, char const *                >
                   || is_same_v< X, string::iterator            >
                   || is_same_v< X, string::const_iterator      >
                   || is_same_v< X, string_view::iterator       >
                   || is_same_v< X, string_view::const_iterator >, "First argument is wrong type" );

    static_assert(    is_same_v< Y, char *                      >
                   || is_same_v< Y, char const *                >
                   || is_same_v< Y, string::iterator            >
                   || is_same_v< Y, string::const_iterator      >
                   || is_same_v< Y, string_view::iterator       >
                   || is_same_v< Y, string_view::const_iterator >, "Second argument is wrong type" );

    // Now we use the constructor which takes a pointer and an integer length
    return std::string_view( static_cast<char const *>( &*a ), static_cast<string_view::size_type>( &*b - &*a ) );
}

template<typename A, typename B>
bool starts_with(A const &a, B const &b)
{
    using std::size_t;
    using std::is_same_v;
    using std::string;
    using std::string_view;

    typedef std::decay_t< std::remove_cv_t< std::remove_reference_t<A> > > X;  // C++17 doesn't have remove_cvref_t
    typedef std::decay_t< std::remove_cv_t< std::remove_reference_t<B> > > Y;

    static_assert(    is_same_v< X, string      >
                   || is_same_v< X, string_view >, "First argument is wrong type" );

    static_assert(    is_same_v< Y, char *       >
                   || is_same_v< Y, char const * >
                   || is_same_v< Y, string       >
                   || is_same_v< Y, string_view  >, "Second argument is wrong type" );

    size_t const sa = a.size();

    size_t sb;

    if constexpr ( is_same_v< Y, char * > || is_same_v< Y, char const * > )
    {
        assert( nullptr != b );
        sb = std::strlen(b);
    }
    else
    {
        sb = b.size();
    }

    if ( sb > sa ) return false;

    return b == a.substr(0u, sb);
}

template<typename A, typename B>
bool ends_with(A const &a, B const &b)
{
    using std::size_t;
    using std::is_same_v;
    using std::string;
    using std::string_view;

    typedef std::decay_t< std::remove_cv_t< std::remove_reference_t<A> > > X;  // C++17 doesn't have remove_cvref_t
    typedef std::decay_t< std::remove_cv_t< std::remove_reference_t<B> > > Y;

    static_assert(    is_same_v< X, string      >
                   || is_same_v< X, string_view >, "First argument is wrong type" );

    static_assert(    is_same_v< Y, char *       >
                   || is_same_v< Y, char const * >
                   || is_same_v< Y, string       >
                   || is_same_v< Y, string_view  >, "Second argument is wrong type" );

    size_t const sa = a.size();

    size_t sb;

    if constexpr ( is_same_v< Y, char * > || is_same_v< Y, char const * > )
    {
        assert( nullptr != b );
        sb = std::strlen(b);
    }
    else
    {
        sb = b.size();
    }

    if ( sb > sa ) return false;

    return b == a.substr(sa - sb);
}

// ==========================================================================
// Section 3 of 9 : Implementation of container type : FIFO map
// ==========================================================================

#include <list>       // list
#include <utility>    // pair
#include <algorithm>  // find
#include <stdexcept>  // out_of_range

template <class T_key_type, class T_mapped_type>
class fifo_map {
public:

    typedef T_key_type    key_type   ;
    typedef T_mapped_type mapped_type;

protected:

    typedef std::pair<key_type,mapped_type> BasicPairType;

    struct PairType : BasicPairType {

        typedef BasicPairType Base;

        PairType(void) : Base() {}

        PairType(key_type const &k) : Base(k, mapped_type()) {}

        template<class T>
        PairType(key_type const &k, T const &m) : Base(k,m) {}

        bool operator==(key_type const &rhs) const
        {
            return this->first == rhs;  // We don't compare the second
        }
    };

    typedef PairType value_type;

    std::list<PairType> data;

public:

    mapped_type &operator[](key_type const &k)
    {
        typename decltype(data)::iterator iter = std::find( data.begin(), data.end(), k );

        if ( iter != data.end() ) return iter->second;

        data.emplace_back(k);

        return data.back().second;
    }

    mapped_type const &at(key_type const &k) const
    {
        typename decltype(data)::const_iterator iter = std::find( data.cbegin(), data.cend(), k );

        if ( iter == data.end() ) throw std::out_of_range("blah blah");

        return iter->second;
    }

    mapped_type &at(key_type const &k)
    {
        return const_cast<mapped_type&>( const_cast<fifo_map const *>(this)->at(k) );
    }

    template<class T>
    void emplace(key_type const &k, T const &m)
    {
        data.emplace_back(k,m);
    }

    std::size_t size(void) const { return data.size(); }

    typename decltype(data)::const_iterator cbegin(void) const { return data.cbegin(); }
    typename decltype(data)::const_iterator cend  (void) const { return data.cend  (); }

    typename decltype(data)::const_iterator begin(void) const { return data.cbegin(); }
    typename decltype(data)::const_iterator end  (void) const { return data.cend  (); }

    typename decltype(data)::iterator begin(void) { return data.begin(); }
    typename decltype(data)::iterator end  (void) { return data.end  (); }
};

// =========================================================================
// Section 4 of 9 : Include standard header files, and define global objects
// =========================================================================

bool constexpr verbose = false;
bool constexpr print_all_scopes = false;

bool only_print_numbers; /* This gets set in main -- don't set it here */

#include <cstdlib>        // EXIT_FAILURE, abort
#include <cstring>        // memset, memcpy
#include <cstdio>         // freopen, stdin
#include <iostream>       // cout, clog, endl
#include <algorithm>      // copy, replace, count, all_of, any_of
#include <iterator>       // next, distance, back_inserter, istream_iterator, iterator_traits
#include <string>         // string, to_string
#include <ios>            // ios::binary
#include <iomanip>        // noskipws
#include <stdexcept>      // out_of_range, runtime_error
#include <utility>        // pair<>
#include <cctype>         // isspace, isalpha, isdigit
#include <list>           // list
#include <map>            // map
#include <array>          // array
#include <tuple>          // tuple
#include <utility>        // pair, move
#include <string_view>    // string_view
#include <set>            // set
#include <regex>          // regex, regex_replace, smatch, match_results
#include <exception>      // exception
#include <chrono>         // duration_cast, milliseconds, steady_clock
#include <sstream>        // ostringstream
#include <atomic>         // atomic<>

using std::cin;
using std::cout;
using std::clog;
using std::endl;
using std::string;
using std::to_string;
using std::string_view;
using std::list;
using std::array;
using std::tuple;
using std::pair;

using std::regex;
using std::regex_replace;
using std::regex_iterator;
using std::smatch;
using std::match_results;

using std::istream_iterator;
using std::back_inserter;

using std::runtime_error;

using std::ostream;
using std::ostringstream;

std::uintmax_t GetTickCount(void)
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

std::uintmax_t g_timestamp_program_start = 0u;

// ==========================================================================
// Section 5 of 9 : regex_top_level_token_iterator
// ==========================================================================

template<
    class BidirIt,
    class CharT = typename std::iterator_traits<BidirIt>::value_type,
    class Traits = std::regex_traits<CharT>
>
class regex_top_level_token_iterator : public std::regex_token_iterator<BidirIt,CharT,Traits> {
private:

    using Base = std::regex_token_iterator<BidirIt,CharT,Traits>;
    Base &base = *static_cast<Base*>(this);

protected:

    BidirIt const _a, _b;  // set in constructor's initialiser list

    typename Base::value_type _strided_match;  /* starts off with matched = false */

    bool Is_Top_Level(void) const
    {
        assert( base != Base() );  // Is_Top_Level should never be called on a "no more matches" token iterator

        size_t counts[4u] = {};  /* (), [], {}, <> */

        for ( BidirIt iter = _a; iter != (*base).second; ++iter )
        {
            switch ( *iter )
            {
            case '(': ++(counts[0u]); continue;
            case ')': --(counts[0u]); continue;

            case '[': ++(counts[1u]); continue;
            case ']': --(counts[1u]); continue;

            case '{': ++(counts[2u]); continue;
            case '}': --(counts[2u]); continue;
            }

            bool const process_next_angle_bracket =
                   0u == counts[0u]
                && 0u == counts[1u]
                && 0u == counts[2u];

            if ( process_next_angle_bracket )
            {
                switch ( *iter )
                {
                case '<': ++(counts[3u]); continue;
                case '>': --(counts[3u]); continue;
                }
            }
        }

        return std::all_of(std::begin(counts),
                           std::end(counts),
                           [](auto const &e){ return 0u == e; });
    }

public:

    regex_top_level_token_iterator(void) : Base(), _a(), _b() {}

    void Keep_Searching_If_Necessary(void)
    {
        for ( _strided_match.matched = false; base != Base(); ++base )
        {
            if ( this->Is_Top_Level() )
            {
                _strided_match.second = (*base).second;  // redundant when _strided_match.matched == false
                return;
            }
            else
            {
                if ( false == _strided_match.matched )
                {
                    _strided_match.matched = true;

                    _strided_match.first = (*base).first;
                }
            }
        }

        _strided_match.matched = false;
    }

    regex_top_level_token_iterator(BidirIt const a, BidirIt const b,
                                   typename Base::regex_type const &re,
                                   int const submatch = 0,
                                   std::regex_constants::match_flag_type const m = std::regex_constants::match_default )
      : Base(a,b,re,submatch,m), _a(a), _b(b)
    {
        Keep_Searching_If_Necessary();
    }

    regex_top_level_token_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        Keep_Searching_If_Necessary();

        return *this;
    }

    bool operator==(regex_top_level_token_iterator const &rhs) const  // Since C++20 we don't need operator!=
    {
        return base == rhs;
    }

    typename Base::value_type const &operator*(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        if ( false == _strided_match.matched )
        {
            return *base;
        }

        return _strided_match;
    }

    typename Base::value_type const *operator->(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        if ( false == _strided_match.matched )
        {
            return base.operator->();
        }

        return &_strided_match;
    }
};

template<
    class BidirIt,
    class CharT = typename std::iterator_traits<BidirIt>::value_type,
    class Traits = std::regex_traits<CharT>
>
class regex_top_level_iterator : public std::regex_iterator<BidirIt,CharT,Traits> {
private:

    using Base = std::regex_iterator<BidirIt,CharT,Traits>;
    Base &base = *static_cast<Base*>(this);

protected:

    bool const _disregard_normal_parentheses;
    BidirIt const _a, _b;  // set in constructor's initialiser list

    bool Is_Top_Level(void) const
    {
        assert( base != Base() );  // Is_Top_Level should never be called on a "no more matches" token iterator

        size_t counts[4u] = {};  /* (), [], {}, <> */

        for ( BidirIt iter = _a; iter != (*base)[0u].first; ++iter )
        {
            switch ( *iter )
            {
            case '(': ++(counts[0u]); continue;
            case ')': --(counts[0u]); continue;

            case '[': ++(counts[1u]); continue;
            case ']': --(counts[1u]); continue;

            case '{': ++(counts[2u]); continue;
            case '}': --(counts[2u]); continue;
            }

            bool const process_next_angle_bracket =
                   0u == counts[0u]
                && 0u == counts[1u]
                && 0u == counts[2u];

            if ( process_next_angle_bracket )
            {
                switch ( *iter )
                {
                case '<': ++(counts[3u]); continue;
                case '>': --(counts[3u]); continue;
                }
            }
        }

        for ( auto const &count : counts )
        {
            if ( _disregard_normal_parentheses && (&count == &counts[0u]) ) continue;

            if ( 0u != count ) return false;
        }

        return true;
    }

public:

    regex_top_level_iterator(void) : Base(), _a(), _b(), _disregard_normal_parentheses(false) {}

    void Keep_Searching_If_Necessary(void)
    {
        for ( ; base != Base(); ++base )
        {
            if ( this->Is_Top_Level() )
            {
                return;
            }
        }
    }

    regex_top_level_iterator(BidirIt const a, BidirIt const b,
                             typename Base::regex_type const &re,
                             std::regex_constants::match_flag_type const m = std::regex_constants::match_default,
                             bool arg_disregard_normal_parentheses = false )
      : Base(a,b,re,m), _a(a), _b(b), _disregard_normal_parentheses(arg_disregard_normal_parentheses)
    {
        Keep_Searching_If_Necessary();
    }

    regex_top_level_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        Keep_Searching_If_Necessary();

        return *this;
    }

    bool operator==(regex_top_level_iterator const &rhs) const  // Since C++20 we don't need operator!=
    {
        return base == rhs;
    }

    typename Base::value_type const &operator*(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        return *base;
    }

    typename Base::value_type const *operator->(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        return base.operator->();
    }
};

using sregex_top_level_token_iterator    = regex_top_level_token_iterator<     string::const_iterator>;
using svregex_top_level_token_iterator   = regex_top_level_token_iterator<string_view::const_iterator>;
using sregex_top_level_iterator          = regex_top_level_iterator      <     string::const_iterator>;
using svregex_top_level_iterator         = regex_top_level_iterator      <string_view::const_iterator>;

using r_sregex_top_level_token_iterator  = regex_top_level_token_iterator<     string::const_reverse_iterator>;
using r_svregex_top_level_token_iterator = regex_top_level_token_iterator<string_view::const_reverse_iterator>;
using r_sregex_top_level_iterator        = regex_top_level_iterator      <     string::const_reverse_iterator>;
using r_svregex_top_level_iterator       = regex_top_level_iterator      <string_view::const_reverse_iterator>;

// ==========================================================================
// Section 6 of 9 : Process keywords and identifiers
// ==========================================================================

inline bool Is_Valid_Identifier_Char(char const c)
{
    return std::isalpha(static_cast<char unsigned>(c)) || std::isdigit(static_cast<char unsigned>(c)) || ('_' == c);
}

inline bool Is_Entire_String_Valid_Identifier(string_view const sv)
{
    assert( false == sv.empty() );

    if ( std::isalpha(static_cast<char unsigned>(sv.front())) || ('_' == sv.front()) )  // First character can't be a digit
    {
        return std::all_of( sv.cbegin(), sv.cend(), [](char const c){ return Is_Valid_Identifier_Char(c); } );
    }
    else
    {
        return false;
    }
}

struct ShortStr {
    char c[4u];
};

inline short unsigned Unique_12_Bit(void)
{
    static std::atomic<short unsigned> retval = -1;

    return ++retval;
}

inline ShortStr Unique_3_Hex_Chars(void)
{
    ShortStr str;

    short unsigned const x = Unique_12_Bit();

    static char const alphabet[] = "0123456789abcdef";

    for ( unsigned i = 0u; i != 3u; ++i )
    {
        str.c[i] = alphabet[ (x >> (8u - 4u*i)) & 0xf ];
    }

    str.c[3u] = '\0';

    return str;
}

char const *const g_strs_keywords[] = {
    "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel",
    "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t",
    "char32_t", "class", "compl", "concept", "const", "consteval",
    "constexpr", "constinit", "const_cast", "continue", "co_await",
    "co_return", "co_yield", "decltype", "default", "delete", "do",
    "double", "dynamic_cast", "else", "enum", "explicit", "export",
    "extern", "false", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept", "not",
    "not_eq", "nullptr", "operator", "or", "or_eq", "private",
    "protected", "public", "reflexpr", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static",
    "static_assert", "static_cast", "struct", "switch", "synchronized",
    "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
};

void Find_And_Erase_All_Keywords(string &s)
{
    for ( auto const &e : g_strs_keywords )
    {
        for ( size_t i = 0u; i < s.size(); ++i )
        {
            i = s.find(e, i);  // e.g. find the word "volatile"

            if ( -1 == i ) break;

            //cout << "Found keyword: " << e << endl;

            size_t const one_past_last = i + strlen(e);

            if ( (one_past_last < s.size()) && Is_Valid_Identifier_Char(s[one_past_last]) )
            {
                //cout << "disregarding" << endl;
                continue;
            }

            if (     (0u != i)       && Is_Valid_Identifier_Char(s[i-1u]) )
            {
                //cout << "disregarding" << endl;
                continue;
            }

            //cout << "= = = = ERASING = = = =" << endl;

            s.erase(i,one_past_last - i);

            --i;  // Because it will be incremented automatically

#if 0
            // Now if there is "\(.*\)" then delete it too

            while ( i < s.size() && std::isspace(s[i]) ) ++i;

            if ( i >= s.size() || '(' != s[i] ) { --i; continue; }

            size_t count = 1u;
            for ( size_t j = i + 1u; j != s.size(); ++j )
            {
                if      ( ')' == s[j] ) --count;
                else if ( '(' == s[j] ) ++count;

                if ( 0u == count )
                {
                    s.erase(i, j - i);
                    break;
                }
            }

            if ( 0u != count ) throw runtime_error("unmatched parentheses after a keyword");
#endif
        }
    }
}

// ==========================================================================
// Section 7 of 9 : Parse function signatures
// ==========================================================================

class Function_Signature {
protected:

    class Parameter {
    protected:

        string _original;
        string _name;  // REVISIT FIX - Maybe see about a string_view here

    public:

        explicit Parameter(string_view const arg) : _original(arg)
        {
            if ( _original.empty() ) throw runtime_error("Function parameter shouldn't be blank here");

            _name = _original;

            while ( false == _name.empty() && false == Is_Entire_String_Valid_Identifier(_name) )
            {
                //_name.remove_prefix(1u);
                _name.erase(0u,1u);
            }

            if ( _name.empty() )
            {
                _name  = "param_";
                _name += Unique_3_Hex_Chars().c;

                _original += ' ';
                _original += _name;
            }
        }

        string_view Name(void) const
        {
            return _name;
        }

        string_view Full(void) const
        {
            return _original;
        }
    };

    string const _original;
    string_view _name;
    fifo_map<size_t,size_t> _found_decltypes;
    list<Parameter> _params;

    void Find_All_Decltypes(string_view const s)
    {
        static regex const my_regex("decltype\\s*\\(");
        svregex_top_level_iterator iter(s.cbegin(), s.cend(), my_regex);

        for ( ; iter != svregex_top_level_iterator(); ++iter )
        {
            size_t const index = (*iter)[0u].second - s.cbegin();

            size_t count = 1u;

            size_t i;
            for ( i = index + 1u; i != s.size(); ++i )
            {
                if      ( ')' == s[i] ) --count;
                else if ( '(' == s[i] ) ++count;

                if ( 0u == count ) break;
            }

            if ( 0u != count ) throw runtime_error("Mismatched parentheses when trying to find decltype's");

            //cout << "=================== Found a decltype =======================" << endl;
            _found_decltypes[index] = i;
        }
    }

    string_view Full_Param_List(void) const
    {
        string_view const s { _original };

        char const *p = &*_name.cend();  // REVISIT FIX - watch out for whitespace, e.g. "int Func (void)"

        //cout << "------------- BAD CHAR = " << *p << "  (name = " << _name << ")" << endl;
        assert( '(' == *p );

        ++p;

        size_t const index = p - &*s.cbegin();

        size_t count = 1u;

        size_t i;
        for ( i = index; i != s.size(); ++i )
        {
            if      ( ')' == s[i] ) --count;
            else if ( '(' == s[i] ) ++count;

            if ( 0u == count ) break;
        }

        if ( 0u != count ) throw runtime_error("Mismatched parentheses when trying to find decltype's");

        return Sv(s.cbegin() + index, s.cbegin() + i);
    }

    void ProcessParams(void)
    {
        _params.clear();

        string_view const full{ Full_Param_List() };

        static regex const my_regex(",");
        svregex_top_level_token_iterator iter(full.cbegin(), full.cend(), my_regex, -1);

        for ( ; iter != svregex_top_level_token_iterator(); ++iter )
        {
            if ( "void" ==  *iter || "" == *iter ) return;  // REVISIT FIX - watch out for whitespace

            _params.emplace_back( Sv(iter->first, iter->second) );

            //cout << "  Parameter Name: " << _params.back().Name() << endl;
        }
    }

public:

    explicit Function_Signature(string_view const arg) : _original(arg)
    {
        string without_keywords{ _original };

        //cout << "Before: " << without_keywords << endl;
        Find_And_Erase_All_Keywords(without_keywords);
        without_keywords.insert(0u," ");
        Find_All_Decltypes(without_keywords);
        //cout << "After: " << without_keywords << endl;

        string_view const s{without_keywords};
        static regex const my_regex("[\\s\\&\\*]([A-z_][A-z_0-9]*)\\s*\\(");

        //cout << "Searching for function name in '" << s << "'" << endl;
        svregex_top_level_iterator iter(s.cbegin(), s.cend(), my_regex, std::regex_constants::match_default, true);

        unsigned count = 0u;
        for ( ; iter != svregex_top_level_iterator(); ++iter )
        {
            //cout << " - - - match - - - " << endl;
            size_t const index_first = (*iter)[0u].first  - s.cbegin();
            size_t const index_last  = (*iter)[0u].second - s.cbegin() - 1u;

            if ( std::any_of(_found_decltypes.cbegin(),
                             _found_decltypes.cend(),
                             [index_first,index_last](auto const &e){ return index_first >= e.first && index_last <= e.second; } ) )
            {
                continue;
            }

            string_view const found { Sv((*iter)[1u].first, (*iter)[1u].second) };

            for ( size_t location = 0u; location < _original.size(); ++location)
            {
                //cout << "Searching for '" << found << "' inside '" << _original << "'" << endl;

                location = _original.find(found, location);

                assert( -1 != location );

                //cout << "Found at location " << location << endl;

                size_t const one_past_last = location + found.size();

                if ( (one_past_last < _original.size()) && Is_Valid_Identifier_Char(_original[one_past_last]) )
                {
                    //cout << "disregarding AAA" << endl;
                    continue;
                }

                if (     (0u != location)       && Is_Valid_Identifier_Char(_original[location-1u]) )
                {
                    //cout << "disregarding BBB because previous char at location " << location - 1u << " is " << s[location - 1u] << endl;
                    continue;
                }

                _name = Sv( _original.cbegin() + location, _original.cbegin() + location + found.size() );

                break;
            }

            //cout << "SECOND " << _name << endl;

            ++count;

            if ( 1u == count ) continue;

            break;
        }

        if ( _name.empty() ) throw runtime_error("Couldn't determine name of function");

        ProcessParams();
    }

    template<class T>
    void Original_Function_Signature_Renamed(T &os) const
    {
        os << Sv( _original.cbegin(), _name.cend() );

        os << "____WITHOUT_CONTINUITY";

        os << Sv( _name.cend(), _original.cend() );
    }

    template<class T>
    void Signature_Of_Replacement_Function(T &os) const
    {
        os << Sv( _original.cbegin(), _name.cend() );

        os << '(';

        for ( auto &e : _params )
        {
            os << e.Full();

            if ( &e != &_params.back() ) os << ", ";
        }

        os << Sv( Full_Param_List().cend(), _original.cend() );
    }

    template<class T>
    void Signature_Of_Replacement_Function____With_Void_Pointer_This(T &os) const
    {
        os << Sv( _original.cbegin(), _name.cend() );

        os << "(void *const arg_this";

        for ( auto &e : _params )
        {
            os << ", ";

            os << e.Full();
        }

        os << Sv( Full_Param_List().cend(), _original.cend() );
    }

    template<class T>
    void Invocation(T &os) const
    {
        os << _name << "(";

        for ( auto const &e : _params )
        {
            os << e.Name();

            if ( &e != &_params.back() )
            {
                os << ", ";
            }
        }

        os << ")";
    }

    template<class T>
    void Invocation____WITH_VOID_POINTER_THIS(T &os, string_view const param_name = "arg_this") const
    {
        os << _name << "(" << param_name;

        for ( auto const &e : _params )
        {
            os << ", ";
            os << e.Name();
        }

        os << ")";
    }

    template<class T>
    void Invocation____WITHOUT_CONTINUITY(T &os) const
    {
        os << _name << "____WITHOUT_CONTINUITY(";

        for ( auto const &e : _params )
        {
            os << e.Name();

            if ( &e != &_params.back() )
            {
                os << ", ";
            }
        }

        os << ")";
    }

    string_view Name(void) const
    {
        return _name;
    }

    string_view Original(void) const
    {
        return _original;
    }
};

// ==========================================================================
// Curly Bracket Manager
// ==========================================================================

string g_intact;

inline void ThrowIfBadIndex(size_t const char_index)
{
    if ( char_index >= g_intact.size() )
        throw runtime_error("Cannot access *(p + " + std::to_string(char_index) + ") inside type char[" + std::to_string(g_intact.size()) + "]");
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

string TextBeforeOpenCurlyBracket(size_t const char_index)  // strips off the template part at the start, e.g. "template<class T>"
{
    ThrowIfBadIndex(char_index);

    if ( 0u == char_index ) return {};

    if ( '{' != g_intact[char_index] ) throw runtime_error("This isn't an open curly bracket!");

    size_t i = char_index;

    while ( --i )
    {
        bool break_out_of_loop = false;

        switch ( g_intact[i] )
        {
        case '}':
        case '{':
        case ';':     // REVISIT FIX - What if we have "for (int i = 0; i != -1; ++i) { . . . }"
        //case '(':
        //case ')':
        //case '<':
        //case '>':

            break_out_of_loop = true;
            break;

        default:

            continue;
        }

        if ( break_out_of_loop ) break;
    }

    string retval = g_intact.substr(i + 1u, char_index - i - 1u);   // REVISIT FIX might overlap

    StringAlgorithms::Minimise_Whitespace(retval);
    StringAlgorithms::replace_all(retval, "::", "mOnKeY");
    StringAlgorithms::replace_all(retval, ":", " : ");
    StringAlgorithms::replace_all(retval, "mOnKeY", "::");

#if 1
    if ( starts_with(retval, "template") ) return {};
#else
    //if ( retval.contains("allocator_traits") ) clog << "1: ===================" << retval << "===================" << endl;
    retval = regex_replace(retval, regex("(template<.*>) (class|struct) (.*)"), "$2 $3");
    retval = regex_replace(retval, regex("\\s*,\\s*"), ",");
    //if ( retval.contains("allocator_traits") ) clog << "2: ===================" << retval << "===================" << endl;
#endif

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
            if ( nullptr == this ) throw runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( (-1 != _indices.first) && (first <= _indices.first) ) throw runtime_error("Open bracket of inner scope must come after open bracket of outer scope");

            _nested.emplace_back(first, this);

            return &_nested.back();
        }

        CurlyPair *Close_Scope_And_Go_Back(size_t const last)
        {
            if ( nullptr == this ) throw runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( nullptr ==        _parent ) throw runtime_error("The root pair is NEVER supposed to get closed");
            if (    last <= _indices.first ) throw runtime_error("Closing backet of inner scope must come after open bracket");

            _indices.second = last;

            return _parent;
        }
    };

protected:

    CurlyPair _root_pair{ (size_t)-1, nullptr};

    void Print_CurlyPair(CurlyPair const &cp, size_t const indentation = 0u) const
    {
        verbose && clog << "- - - - - Print_CurlyPair( *(" << &cp << "), " << indentation << ") - - - - -" << endl;

        string str;

        extern tuple< string,string, list< array<string,3u> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp);

        if ( false == only_print_numbers )
        {
            str = std::get<1u>(Intro_For_Curly_Pair(cp));
        }

        if ( false == str.empty() || print_all_scopes )
        {
            for ( size_t i = 0u; i != indentation; ++i )
            {
                clog << "    ";
            }

            clog << cp.First() << " (Line #" << LineOf(cp.First())+1u << "), " << cp.Last() << " (Line #" << LineOf(cp.Last())+1u << ")";

            verbose && clog << "  [Full line: " << TextBeforeOpenCurlyBracket(cp.First()) << "]";

            if ( false == only_print_numbers )
            {
                extern string GetNames(CurlyBracketManager::CurlyPair const &);

                //clog << "    " << GetNames(cp);

                clog << "  [" << GetNames(cp) << "]";
            }

            clog << endl;
        }

        for ( CurlyPair const &e : cp.Nested() )
        {
            verbose && clog << "    - - - About to recurse" << endl;
            Print_CurlyPair(e, indentation + 1u);
        }
    }

public:

    struct ParentError : std::exception { };

    void Process(void)
    {
        verbose && clog << "========= STARTING PROCESSING ===========" << endl;

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

        verbose && clog << "========= ENDING PROCESSING ===========" << endl;
    }

    void Print(void) const
    {
        for ( CurlyPair const &e : _root_pair.Nested() )
        {
            verbose && clog << "    - - - About to recurse" << endl;
            Print_CurlyPair(e);
        }
    }

} g_curly_manager;

struct Method_Info {
    string::const_iterator iter_first_char;
    Function_Signature fsig;
    CurlyBracketManager::CurlyPair const *p_body = nullptr;
    ostringstream replacement_body;

    Method_Info(string_view const sv) : fsig(sv) {}

    Method_Info(void) : fsig("int Dummy(int)") {}
};

fifo_map< string, fifo_map<size_t, Method_Info> > g_func_alterations_all;

string_view Indentation_For_CurlyPair(CurlyBracketManager::CurlyPair const &cp)
{
    /* Two examples:

    (1)

        "    void Func(void)\n"
        "    {\n"

    (2)

        "    void Func(void) {\n"


    Step 1: Decrement from the location of '{' until you find either '\n' or start of file

    Step 2: Having found the start of the line, move forward until you encounter non-whitespace

    */

    size_t i = cp.First();

    assert( '{' == g_intact[i] );

    for ( ; -1 != i && '\n' != g_intact[i]; --i );

    ++i;

    /* When control reaches here, g_intact[i] is either:
       (1) The first char in the file
       (2) The first char on the line
    */

    size_t const i_first_char = i;

    auto WhiteOtherThanNewLine = [](char const c) -> bool
    {
        return '\n' != c && std::isspace( static_cast<char unsigned>(c) );
    };

    for ( i = i_first_char; WhiteOtherThanNewLine(g_intact[i]); ++i );

    return Sv( &g_intact[i_first_char], &g_intact[i] );  // REVISIT FIX - possible dereference of invalid iterator
}

// ==========================================================================
// Section 8 of 9 : Generate the auxillary code needed for Continuity Methods
// ==========================================================================

void Print_Helper_Classes_For_Class(string classname)
{
    cout << "// ==========================================================================\n"
            "// Helper classes for continuity methods within class " << classname << "\n"
            "// ==========================================================================\n\n";

    fifo_map<size_t, Method_Info> const &g_func_alterations = g_func_alterations_all.at(classname);

    StringAlgorithms::replace_all(classname, "::", "_scope_");

    cout << "namespace Continuity_Methods { namespace Helpers { namespace " << classname << " {\n\n";

    cout << "namespace Testers {\n";
    for ( auto const &e : g_func_alterations )
    {
        string_view const tmp1 = e.second.fsig.Name();

        cout << "    template<class U, class = decltype(&U::" << tmp1 << "____WITHOUT_CONTINUITY)>\n"
             << "    struct " << tmp1 << "____WITHOUT_CONTINUITY {};\n\n";

        cout << "    template<class U, class = decltype(&U::" << tmp1 << ")>\n"
             << "    struct " << tmp1 << " {};\n\n";
    }
    cout << "}\n\n";

    cout <<
    "class IMethodInvoker {\n"
    "protected:\n"
    "\n"
    "    // All methods have one extra\n"
    "    // parameter for 'this' as 'void*'\n";

    for ( auto const &e : g_func_alterations )
    {
        cout << "    virtual ";

        e.second.fsig.Signature_Of_Replacement_Function____With_Void_Pointer_This(cout);

         cout << " = 0;\n";
    }

    cout <<
    "\n    friend class Invoker;\n"
    "};\n\n";

    cout <<
    "template<class Derived>\n"
    "struct MethodInvoker final {\n"
    "\n"
    "    template<class Base>\n"
    "    class MI final : public IMethodInvoker {\n"
    "    protected:\n"
    "        // All methods have one extra\n"
    "        // parameter for 'this' as 'void*'\n";

    for ( auto const &e : g_func_alterations )
    {
        string_view const name = e.second.fsig.Name();

        cout << "        ";

        e.second.fsig.Signature_Of_Replacement_Function____With_Void_Pointer_This(cout);

        cout << " override\n" <<
                "        {\n"
                "            Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));\n"
                "\n"
                "            if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::" << classname << "::Testers::" << name << "____WITHOUT_CONTINUITY>::value )\n"
                "            {\n"
                "                return p->Base::";

        e.second.fsig.Invocation____WITHOUT_CONTINUITY(cout);

        cout << ";\n"
                "            }\n"
                "            else if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::" << classname << "::Testers::" << name << ">::value )\n"
                "            {\n"
                "                return p->Base::";

        e.second.fsig.Invocation(cout);

        cout << ";\n"
                "            }\n"
                "            else\n"
                "            {\n"
                "                return;\n"
                "            }\n"
                "        }\n\n";
    }

    cout << "    };\n"
            "};\n\n";

    cout <<
    "class Invoker final {\n"
    "protected:\n"
    "\n"
    "    IMethodInvoker &_mi;\n"
    "    void *const _this;\n"
    "\n"
    "public:\n"
    "\n"
    "    Invoker(IMethodInvoker &arg_mi, void *const arg_this)\n"
    "      : _mi(arg_mi), _this(arg_this) {}\n\n";

    for ( auto const &e : g_func_alterations )
    {
        cout << "    // The extra 'this' parameter is no longer needed\n"
             << "    ";

        e.second.fsig.Signature_Of_Replacement_Function(cout);

        cout << " // not virtual\n"
             << "    {\n"
             << "        return _mi.";

        e.second.fsig.Invocation____WITH_VOID_POINTER_THIS(cout, "_this");

        cout << ";\n"
             << "    }\n\n";
    }

    cout << "};\n\n";

    cout << "}}}  // close three namespaces\n\n";
}

// =============================================================
// Section 9 of 9 : Parse all the class definitions in the input
// =============================================================

void Replace_All_String_Literals_With_Spaces(bool undo = false)
{
    static fifo_map<size_t, string> strlits;

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

void Replace_All_Preprocessor_Directives_With_Spaces(bool undo = false)
{
    static fifo_map<size_t, string> directives;

    if ( undo )
    {
        for ( auto const &e : directives )
        {
            std::memcpy(&g_intact[e.first], &e.second.front(), e.second.size());
        }

        return;
    }

    for ( size_t i = 0u; i != g_intact.size(); ++i )
    {
        assert( '\r' != g_intact[i] );
        //assert( '\t' != g_intact[i] );

        if ( '#' == g_intact[i] )
        {
            if ( (0u == i) || ('\n' == g_intact[i-1u]) )
            {
                if ( g_intact.size() == (i + 1u) )
                {
                    // The hash symbol is the last char in the file
                    directives[i] = "#";
                    return;
                }

                // We have a preprocessor directive starting at index i
                size_t j = g_intact.find_first_of('\n', i + 1u);

                if ( -1 == j ) j = g_intact.size() - 1u;

                directives[i] = g_intact.substr(i, j - i);

                std::memset(&g_intact[i], ' ', j - i);

                i = j;  // i will be incremented on the next 'for' iterator
            }
        }
    }

    clog << "Preprocessor Replacements:\n"
            "==========================\n";

    for ( auto const &e : directives )
    {
        clog << "Index " << e.first << ", Len = " << e.second.size() << ", [" << e.second << "]" << endl;
    }
}

CurlyBracketManager::CurlyPair const *CurlyBracketManager::CurlyPair::Parent(void) const
{
    if ( nullptr == this          ) throw runtime_error("The 'this' pointer in this method is a nullptr!");
    if ( nullptr == this->_parent ) throw runtime_error("Parent() should never be invoked on the root pair");

    if ( &g_curly_manager._root_pair == this->_parent ) throw CurlyBracketManager::ParentError();

    return this->_parent;
}

fifo_map< string, tuple< list<CurlyBracketManager::CurlyPair const *>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation
/* For example:

         e.first "class"
get<0>(e.second) { pointer to first '{', pointer to second '{', pointer to third '{' }
get<1>(e.second) "Laser_NitrogenPicoSecond"
get<2>(e.second) {
                     { "", "public", "Laser_Nitrogen"   }
                     { "", "public", "Laser_PicoSecond" }
                     { "virtual", "protected", "NoSuchClass }
                 }
*/

list< array<string,3u> > Parse_Bases_Of_Class(string const &str)
{
    array<string,3u> tmp;

    list< array<string,3u> > retval;

    regex const my_regex (",");

    regex const my_regex2("\\s");

    for (sregex_top_level_token_iterator iter(str.begin(), str.end(), my_regex, -1);
         iter != sregex_top_level_token_iterator();
         ++iter)
    {
        string base_info_str{ *iter };

        for (sregex_top_level_token_iterator iter2(base_info_str.begin(), base_info_str.end(), my_regex2, -1);
             iter2 != sregex_top_level_token_iterator();
             ++iter2)
        {
            string word { *iter2 };

            StringAlgorithms::Minimise_Whitespace(word);

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

        //clog << "[ADD RECORD : " << std::get<0u>(tmp) << " : " << std::get<1u>(tmp) << " : " << std::get<2u>(tmp) << "]";
        retval.push_back(tmp);
    }

    return retval;
}

tuple< string, string, list< array<string,3u> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp)
{
    if ( cp.First() >= g_intact.size() || cp.Last() >= g_intact.size() )
    {
        throw runtime_error( string("Curly Pair is corrupt [") + to_string(cp.First()) + "," + to_string(cp.Last()) + "]" );
    }

    string intro = TextBeforeOpenCurlyBracket(cp.First());

    if ( starts_with(intro, "namespace") )
    {
        string str;

        regex const my_regex (" ");

        unsigned i = 0u;
        for (sregex_top_level_token_iterator iter(intro.begin(), intro.end(), my_regex, -1);
             iter != sregex_top_level_token_iterator();
             ++iter)
        {
            if ( 1u != i++ ) continue;

            str = *iter;

            break;
        }

        return { "namespace", str, {} };
    }

    if (   !(starts_with(intro,"class") || starts_with(intro,"struct"))   ) return {};

    StringAlgorithms::erase_all( intro, " final" );   // careful it might be "final{" REVISIT FIX any whitespace not just space

    // The following finds spaces except those found inside angle brackets
    regex const my_regex("\\s");

    sregex_top_level_token_iterator iter(intro.begin(), intro.end(), my_regex, -1);

    assert( iter != sregex_top_level_token_iterator() );  // This should never happen (takes "class" from "class __cxx11::collate : public locale::facet")

    ++iter;

    if ( sregex_top_level_token_iterator() == iter )
    {
        // Control reaches here if we have an anonymous struct (e.g. inside a function or inside a parent struct)
        return {};
    }

    string const str{ *iter }; // takes "__cxx11::collate" from "class __cxx11::collate : public locale::facet"

    if ( ++iter == sregex_top_level_token_iterator() ) return { "class", str, {} };  // This bring us to the sole colon
    if ( ++iter == sregex_top_level_token_iterator() ) return { "class", str, {} };  // This brings it to the first word after the colon (e.g. virtual)

    return { "class", str, Parse_Bases_Of_Class( string( &*(iter->first) ) ) };  // REVISIT FIX might be 'struct' instead of 'class' (public Vs private)
}

string GetNames(CurlyBracketManager::CurlyPair const &cp)
{
    tuple< string, string, list< array<string,3u> >  > tmppair = Intro_For_Curly_Pair(cp);

    string retval = std::get<1u>(tmppair);  /* e.g. 0 = class, 1 = Laser, 2 = [ ... base classes ... ] */

    if ( retval.empty() ) return {};

    size_t iteration = 0u;

    try
    {
        for ( CurlyBracketManager::CurlyPair const *p = &cp; p = p->Parent(); /* no post-processing */ )  // will throw exception when root pair is reached
        {
            //clog << "Iteration No. " << iteration << endl;

            verbose && clog << " / / / / / / About to call Word_For_Curly_Pair(" << p->First() << ", " << p->Last() << ")" << endl;
            string const tmp = std::get<1u>( Intro_For_Curly_Pair(*p) );
            verbose && clog << " / / / / / / Finished calling Word_For_Curly_Pair" << endl;

            if ( tmp.empty() ) continue;

            retval.insert(0u, tmp + "::");
        }
    }
    catch(CurlyBracketManager::ParentError const &)
    {
        //clog << "********** ParentError ************";
    }

    retval.insert(0u, "::");

    //fifo_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    std::get<0u>( g_scope_names[retval] ).push_back(&cp);  // Note that these are pointers

    std::get<1u>( g_scope_names[retval] ) = std::get<0u>(tmppair);

    std::get<2u>( g_scope_names[retval] ) = std::get<2u>(tmppair);

    return retval;
}

size_t Find_Last_Double_Colon_In_String(string_view const s)
{
    static regex const r("[:](?=[^\\<]*?(?:\\>|$))");  // matches a semi-colon so long as it's not enclosed in angle brackets

    match_results<string_view::const_reverse_iterator> my_match;

    if ( regex_search(s.crbegin(), s.crend(), my_match, r) )  // returns true if there is at least one match
    {
        size_t const index_of_second_colon_in_last_double_colon = &*(my_match[0u].first) - &s.front(); // REVISIT FIX - consider using my_match.position() here
        assert( ':' == s[index_of_second_colon_in_last_double_colon] );

        size_t const index_of_first_colon_in_last_double_colon  = index_of_second_colon_in_last_double_colon - 1u;
        if ( ':' != s[index_of_first_colon_in_last_double_colon] ) throw runtime_error("String should only contain a double-colon pair, but it contains a lone colon");

        //cout << "============ POSITION = " << index_of_first_colon_in_last_double_colon << " =================" << endl;

        return index_of_first_colon_in_last_double_colon;
    }

    return -1;
}

size_t Find_Second_Last_Double_Colon_In_String(string_view const s)
{
    static regex const r("(\\<.*\\>)|::");  // matches a double semi-colon so long as it's not enclosed in angle brackets

    bool first = true;

    using r_svregex_iterator = regex_iterator<string_view::const_reverse_iterator>;

    for(r_svregex_iterator iter  = r_svregex_iterator(s.crbegin(), s.crend(), r);
                           iter != r_svregex_iterator();
                           ++iter )
    {
        if ( first )
        {
            first = false;
            continue;
        }

        match_results<string_view::const_reverse_iterator> my_match = *iter;

        size_t const index_of_first_colon_in_last_double_colon = &*(my_match[0u].first) - &s.front() - 1u; // REVISIT FIX - consider using my_match.position() here

        assert( ':' == s[index_of_first_colon_in_last_double_colon     ] );
        assert( ':' == s[index_of_first_colon_in_last_double_colon + 1u] );

        //cout << "============ POSITION = " << index_of_first_colon_in_last_double_colon << " =================" << endl;

        return index_of_first_colon_in_last_double_colon;
    }

    return -1;
}

bool Strip_Last_Scope(string &str)
{
    /*
    Change the prefix from:
        ::std::__allocator_traits_base::__rebind<_Tp,_Up,__void_t<typename _Tp::template rebind<_Up>::other>>::
    to:
        ::std::__allocator_traits_base::
    */

    assert( false == str.empty() );

    char const separator[] = "::";

    size_t constexpr seplen = sizeof(separator) - 1u;

    if ( separator == str ) return false; // If the input is "::"

    if ( (str.size() < (2*seplen + 1u)) || (false == starts_with(str,separator)) || (false == ends_with(str,separator)) )  // minimum = "::A::"
    {
        throw runtime_error("Remove_Last_Scope: Invalid string");
    }

    regex const double_colon(separator);  // REVISIT FIX watch out for control characters interpretted as a regex formula

    r_sregex_top_level_iterator iter( str.crbegin(), str.crend(), double_colon );  // reverse

    assert( r_sregex_top_level_iterator() != iter );  // cppcheck-suppress assertWithSideEffect

    ++iter;  // skip the first match because it's the trailing "::"

    assert( r_sregex_top_level_iterator() != iter );  // cppcheck-suppress assertWithSideEffect

    str.resize(  str.size() - std::distance(str.crbegin(), (*iter)[0u].first)  );

    return true;
}

fifo_map<string,string> g_psuedonyms;

string Find_Class_Relative_To_Scope(string &prefix, string classname)
{
    assert( false == (prefix.empty() && classname.empty()) );

    decltype(g_scope_names)::mapped_type const *p = nullptr;

    if ( starts_with(classname,"::") )  // If it's an absolute class name rather than relative
    {
        g_scope_names.at(classname);  // just to see if it throws
        return classname;
    }

    if ( -1 != prefix.find("<") || -1 != classname.find("<") ) return {};  // Don't tolerate any templates

    auto Adjust_Class_Name = [](string &arg_prefix, string &arg_classname) -> void
    {
        string const what_we_looked_up{ arg_prefix + arg_classname };

        try
        {
            arg_classname = g_psuedonyms.at(what_we_looked_up);
            arg_prefix.clear();
        }
        catch(std::out_of_range const &e) {}

        if ( arg_prefix.empty() && arg_classname.empty() )
        {
            throw runtime_error("Look-up of '" + what_we_looked_up + "' came back blank");
        }
    };

    string const intact_prefix{ prefix };

    string full_name;

    for (; /* ever */ ;)
    {
        if ( std::count(prefix.cbegin(),prefix.cend(),'>') != std::count(prefix.cbegin(),prefix.cend(),'<') )
        {
            string tmp("Aborting because prefix has uneven angle brackets - (");
            tmp += prefix;
            tmp += ")\n";

            cout << tmp;
            clog << tmp;
            std::cerr << tmp;
            std::abort();
        }

        Adjust_Class_Name(prefix, classname);

        full_name  = prefix;
        full_name += classname;

        try
        {
            p = &( g_scope_names.at(full_name) );
        }
        catch (std::out_of_range const &)
        {
            clog << " - - - FIRST FAILED - - - Prefix='" << prefix << "', Classname='" << classname << "' - Fullname='" << full_name << "'" << endl;

            string class_name_without_template_specialisation = regex_replace( string(classname), regex("<.*>"), "");  // REVISIT FIX -- gratuitous memory allocations

            if ( class_name_without_template_specialisation != classname )
            {
                string duplicate_original_full_name{ full_name };  // not const because we std::move() from it later

                Adjust_Class_Name(prefix, class_name_without_template_specialisation);

                full_name  = prefix;
                full_name += class_name_without_template_specialisation;

                try
                {
                     p = &( g_scope_names.at(full_name) );
                }
                catch (std::out_of_range const &)
                {
                    clog << " - - - SECOND FAILED - - - Prefix='" << prefix << "', Classname='" << class_name_without_template_specialisation << "' - Fullname='" << full_name << "'" << endl;

                    full_name = std::move(duplicate_original_full_name);
                }
            }
        }

        if ( nullptr != p )
        {
            //clog << "Success: found '" << string(classname) << "') as ('" << full_name << "')";
            break;  // If we already have found the class then no need to keep searching
        }

        // Last resort: Change the prefix from "::std::__cxx11" into "::std", so that "::std::__cxx11::locale::facet" becomes "::std::locale::facet"
        if ( false == Strip_Last_Scope(prefix) ) break;
    }

    if ( nullptr == p )
    {
        //throw std::out_of_range("Encountered a class name that hasn't been defined ('" + string(classname) + "') referenced inside ('" + string(intact_prefix) + "')");

        clog << "WARNING: Cannot find base class '" + string(classname) + "' referenced inside '" + string(intact_prefix) + "'" << endl;

        return {};
    }

    return full_name;
}

bool Recursive_Print_All_Bases_PROPER(string prefix, string classname, std::set<string> &already_recorded, bool is_virtual, list<string> &retval)
{
    decltype(g_scope_names)::mapped_type const *p = nullptr;

    auto Adjust_Class_Name = [](string &arg_prefix, string &arg_classname) -> void
    {
        try
        {
            arg_classname = g_psuedonyms.at(arg_prefix + arg_classname);
            arg_prefix.clear();
        }
        catch(std::out_of_range const &e) {}
    };

    Adjust_Class_Name(prefix, classname);

    string const full_name = Find_Class_Relative_To_Scope(prefix, classname); // This will throw if class is unknown

    if ( full_name.empty() ) return false;  // not a fatal error if we can't find a base class that's a template class

    bool const is_new_entry = already_recorded.insert(full_name).second;  // set::insert returns a pair, the second is a bool saying if it's a new entry

    if ( false == is_new_entry && true == is_virtual ) return false;  // if it's not a new entry and if it's virtual

    for ( auto const &e : std::get<2u>( g_scope_names.at(full_name) ) )
    {
        string const &base_name = std::get<2u>(e);

        if constexpr ( verbose )
        {
            clog << " [ALREADY_SEEN=";
            for ( auto const &e_already : already_recorded ) clog << e_already << ", ";
            clog << "] ";
            clog << " [[[VIRTUAL=" << (("virtual" == std::get<0u>(e)) ? "true]]]" : "false]]] ") << endl;
        }

        size_t const index_of_last_colon = Find_Last_Double_Colon_In_String(classname);

        if ( -1 != index_of_last_colon && (':' != classname[0u]) )  /* Maybe it's like this:   Class MyClass : public ::SomeClass {}; */
        {
            // This deals with the case of a class inheriting from 'std::runtime_error', which inherits from 'exception' instead of 'std::exception'
            prefix += classname.substr(0u, index_of_last_colon);
            prefix += "::";
        }

        if ( Recursive_Print_All_Bases_PROPER(prefix, base_name, already_recorded, "virtual" == std::get<0u>(e), retval) )
        {
            retval.push_back(prefix + base_name);
        }
    }

    return true;
}

list<string> Get_All_Bases(string_view arg)
{
    size_t const index_of_last_colon = Find_Last_Double_Colon_In_String(arg);

    if ( -1 == index_of_last_colon ) throw runtime_error("There's no double-colon in the argument to Get_All_Bases");

    string_view const prefix    = arg.substr(0u, index_of_last_colon + 2u);
    string_view const classname = arg.substr(index_of_last_colon +  2u);

    std::set<string> already_recorded;

    list<string> retval;

    Recursive_Print_All_Bases_PROPER(string(prefix), string(classname), already_recorded, false, retval);

    return retval;
}

list< pair<size_t,size_t> > GetOpenSpacesBetweenInnerCurlyBrackets(CurlyBracketManager::CurlyPair const &cp)
{
    assert( cp.Last() > cp.First() );

    // Check 1: To see if there's nothing between the curly brackets
    if ( cp.Last() == (cp.First() + 1u) ) return {};   // e.g. if we have "class Monkey {};"

    // Check 2: To see if there's no inner curly brackets
    if ( cp.Nested().empty() )
    {
        return { { cp.First() + 1u, cp.Last() - 1u }  }; // e.g. if we have "class Monkey { int i; };"
    }

    // If control reaches here, we have nested curly brackets

    list< pair<size_t,size_t> > retval;

    list<CurlyBracketManager::CurlyPair>::const_iterator iter = cp.Nested().cbegin();

    size_t begin_from = cp.First() + 1u;

    for ( auto const &e : cp.Nested() )
    {
        assert( '{' == g_intact[e.First()] );
        assert( '}' == g_intact[e.Last() ] );

        assert( e.First() >= begin_from );

        // Check 3: If we have class Monkey { void Func(void) {{}} };
        if ( begin_from == e.First() ) continue;

        // If control reaches here, there's an open space to record
        retval.push_back( { begin_from, e.First() - 1u } );  // fist and second can be equal here if just one char between '}' and '{'

        begin_from = e.Last() + 1u;

        if ( begin_from >= g_intact.size() )
        {
            cout << "About to abort because begin_from >= g_intact.size()\n" << endl;
            clog << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::cerr << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::abort();
        }
    }

    assert( cp.Last() >= begin_from );

    if ( cp.Last() > begin_from ) // must rule out "namespace Dog { namespace Cat { int i; }}"
    {
        retval.push_back( { begin_from, cp.Last() - 1u } );
    }

    return retval;
}

void Find_All_Usings_In_Open_Space(size_t const first, size_t const last, string scope_name)
{
    assert( last >= first );  // It's okay for them to be equal if we have "{ }"

    assert( ends_with(scope_name,"::") );

    regex r("using[\\s]+(.+)[\\s]*=[\\s]*(.+)[\\s]*;");

    for(std::sregex_iterator i  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             i != std::sregex_iterator();
                             ++i )
    {
        string impersonator = regex_replace( i->str(), r, "$1" );
        string original     = regex_replace( i->str(), r, "$2" );

        impersonator = regex_replace(impersonator, regex("typename\\s*"), "");
        impersonator = regex_replace(impersonator, regex("template\\s*"), "");
        original = regex_replace(original, regex("typename\\s*"), "");
        original = regex_replace(original, regex("template\\s*"), "");

        StringAlgorithms::Minimise_Whitespace(impersonator);
        StringAlgorithms::Minimise_Whitespace(original    );

        try
        {
            string full_name_of_original = Find_Class_Relative_To_Scope(scope_name, original);  // might throw out_of_range
            //cout << "Old = " << original << ", New = " << impersonator << endl;
            if ( full_name_of_original.empty() ) throw std::out_of_range("");
            g_psuedonyms[scope_name + impersonator] = full_name_of_original;
        }
        catch (std::out_of_range const &e)
        {
            clog << "====== WARNING: When parsing 'using' declaration, cannot find class '" << original << "' relative to scope '" << scope_name << "'";
            return;
        }
    }

    r = "typedef\\s+(.+?)\\s+(.*)\\s*;";

    for(std::sregex_iterator i  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             i != std::sregex_iterator();
                             ++i )
    {
        //clog << "Iterating in typedef loop" << endl;

        string impersonator = regex_replace( i->str(), r, "$2" );
        string original     = regex_replace( i->str(), r, "$1" );

        impersonator = regex_replace(impersonator, regex("typename\\s*"), "");
        impersonator = regex_replace(impersonator, regex("template\\s*"), "");
        original = regex_replace(original, regex("typename\\s*"), "");
        original = regex_replace(original, regex("template\\s*"), "");

        StringAlgorithms::Minimise_Whitespace(impersonator);
        StringAlgorithms::Minimise_Whitespace(original    );

        try
        {
            string full_name_of_original = Find_Class_Relative_To_Scope(scope_name, original);  // might throw out_of_range
            //cout << "Old = " << original << ", New = " << impersonator << endl;
            if ( full_name_of_original.empty() ) throw std::out_of_range("");
            g_psuedonyms[scope_name + impersonator] = full_name_of_original;
        }
        catch (std::out_of_range const &e)
        {
            clog << "====== WARNING: When parsing 'typedef' declaration, cannot find class '" << original << "' relative to scope '" << scope_name << "'";
            return;
        }
    }
}

// The following map is as follows:
//        first  == index of where to insert "____WITHOUT_CONTINUITY"
// second.first  == CurlyPair const *
// second.second == entire text of new function

CurlyBracketManager::CurlyPair const &Find_Curly_Pair_For_Function_Body_In_Class(CurlyBracketManager::CurlyPair const &parent, size_t const index)
{
    for ( auto const &e : parent.Nested() )
    {
        if ( e.First() == index ) return e;
    }

    throw runtime_error("Cannot find function body for method marked continue");
}

class IndentedOstream {
public:

    class Indent   {};
    class Unindent {};

protected:

    bool            is_first_time = true;
    unsigned        next_indentation = 1u;
    string   const  str_white;
    std::ostream   &os;

    void PrintIndentation(void)
    {
        os << str_white;

        for ( unsigned i = 1u; i != next_indentation; ++i )
        {
            os << "    ";
        }
    }

public:

    IndentedOstream(std::ostream &arg_os, string_view const arg_sv) : os(arg_os), str_white(arg_sv) {}

    template<typename ArgType>
    IndentedOstream &operator<<(ArgType const &arg)
    {
        if ( is_first_time )
        {
            PrintIndentation();
            is_first_time = false;
        }

        os << arg;

#if 0
        if ( static_cast<void*>(&arg) == static_cast<void*>(&endl< char,std::char_traits<char> >) )
        {
            PrintIndentation();
        }
#endif

        return *this;
    }

    IndentedOstream &operator<<(char const *p)
    {
        static char buf[1024u];

        if ( is_first_time )
        {
            PrintIndentation();
            is_first_time = false;
        }

        for ( char const *q = p; '\0' != *q; ++q )
        {
            if ( '\n' == *q )
            {
                memcpy(buf, p, q-p + 1u);  // could be equivalent to memcpy(buf, "{\n", 2u); or even memcpy(buf, "\n", 1u);

                buf[q-p] = '\0';

                //strcpy(buf, "[]");

                //clog << "\nPutting in stream [" << buf << "]" << endl;

                os << buf << endl;

                PrintIndentation();

                p = q + 1u;
            }
        }

        //clog << "\nPutting in stream [" << p << "]" << endl;
        os << p;

        return *this;
    }

    IndentedOstream &operator<<(string const &arg)
    {
        return *this << arg.c_str();
    }

    IndentedOstream &operator<<(Indent const &)
    {
        ++next_indentation;

        return *this;
    }

    IndentedOstream &operator<<(Unindent const &)
    {
        --next_indentation;

        return *this;
    }
};

bool Find_All_Methods_Marked_Continue_In_Class(string_view const svclass, CurlyBracketManager::CurlyPair const &cp, size_t const first, size_t const last)
{
    using Indent   = IndentedOstream::Indent;
    using Unindent = IndentedOstream::Unindent;

    assert( last >= first );  // It's okay for them to be equal if we have "{ }"

    bool retval = false;

    regex r("([A-z_][A-z_0-9]*)\\s+([A-z_][A-z_0-9]*)\\s*\\((.*)\\)\\s*.*(continue)\\s*(;|)");

    for(std::sregex_iterator iter  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             iter != std::sregex_iterator();
                             ++iter )
    {
        clog << "At least one continue method found" << endl;

        retval = true;

        fifo_map<size_t, Method_Info> &g_func_alterations = g_func_alterations_all[string(svclass)];

        size_t const index = ((*iter)[2u]).second - g_intact.cbegin() - 1u;  // REVISIT FIX - corner cases such as '{}'

        g_func_alterations.emplace( index, string((*iter)[1u]) + " " + string((*iter)[2u]) + "(" + string((*iter)[3u]) + ")" );  // create new element

        g_func_alterations[index].iter_first_char = (*iter)[0u].first;

        //g_func_alterations[index].fsig = Function_Signature(string((*iter)[1u]) + " " + string((*iter)[2u]) + "(" + string((*iter)[3u]) + ")");

        // The loop on the next line replaces "continue" with "        "
        for ( char *p = const_cast<char*>(&*(((*iter)[4u]).first)); p != &*(((*iter)[4u]).second); ++p )  // const_cast is fine here REVISIT FIX possible dereference null pointer
        {
            *p = ' ';
        }

        CurlyBracketManager::CurlyPair const &cp_body = Find_Curly_Pair_For_Function_Body_In_Class(cp, ((*iter)[5u]).second - g_intact.cbegin());
        g_func_alterations[index].p_body = &cp_body;  // REVISIT FIX - corner cases such as '{}'

        if ( ';' == *((*iter)[5u]).first ) throw runtime_error("Can only parse inline member functions within the class definition");

        list<string> bases{ Get_All_Bases(svclass) };

        if ( bases.empty() ) throw runtime_error("Method marked continue inside a class that has no base classes");

        string derived_replaced(svclass);
        StringAlgorithms::replace_all(derived_replaced, "::", "_scope_");

        IndentedOstream s( g_func_alterations[index].replacement_body, Indentation_For_CurlyPair(cp_body) );

        s << "\n";

        g_func_alterations[index].fsig.Signature_Of_Replacement_Function(s);

        s << "\n{";

        s << Indent();

        s << "\nusing namespace Continuity_Methods::Helpers::" << derived_replaced << ";\n\n";

        s << "using MethodInvokerT = MethodInvoker<" << svclass << ">;\n\n";

        for ( auto &e : bases )
        {
            s << "static MethodInvokerT::MI< " << e << " > mi_";

            StringAlgorithms::replace_all(e, "::", "_scope_");

            s << e << ";\n";
        }

        s << "\nInvoker methods[" << bases.size() << "u] = {";
        s << Indent();
        for ( auto &e : bases )
        {
            StringAlgorithms::replace_all(e, "::", "_scope_");

            s << "\nInvoker(mi_" << e << ", " << "this),";
        }

        s << Unindent();

        s << "\n};\n\n";

        s << "for ( auto &e : methods | Continuity_Methods::constructor_order )\n"
             "{\n"
             "    e.";

        g_func_alterations[index].fsig.Invocation(s);

        s << ";\n"
             "}\n\n";

        s << "this->";

        g_func_alterations[index].fsig.Invocation____WITHOUT_CONTINUITY(s);

        s << ";\n";

        s << Unindent();

        s << "\n}\n\n";
    }

    return retval;
}

void Instantiate_Scope_By_Scope_Where_Necessary(string_view str)
{
    assert( false == str.empty() );

    char const separator[] = "::";

    size_t constexpr seplen = sizeof(separator) - 1u;

    if ( separator == str ) return; // If the input is "::"

    if ( ends_with(str,separator) ) str.remove_suffix(seplen);  // Turn "::A::B::" into "::A::B"

    if ( (str.size() < (seplen + 1u)) || (false == starts_with(str,separator)) || ends_with(str,separator) )  // minimum = "::A"
    {
        throw runtime_error("Remove_Last_Scope: Invalid string");
    }

    regex const double_colon(separator);  // REVISIT FIX watch out for control characters interpretted as a regex formula

    size_t i = -2;

    for ( svregex_top_level_token_iterator iter( str.cbegin(), str.cend(), double_colon, -1 );
          svregex_top_level_token_iterator() != iter;
          ++iter )
    {
        ++i;

        if ( -1 == i ) continue;

        clog << "Level " << i << ": " << Sv(str.cbegin(), iter->second);

        if ( -1 != Sv(iter->first,iter->second).find('<') )  // check if right-most is a template class
        {
            clog << "    <---- template class";
        }

        clog << endl;
    }
}

void Print_Final_Output(void)
{
    Replace_All_String_Literals_With_Spaces(true);
    Replace_All_Preprocessor_Directives_With_Spaces(true);

    size_t i = 0u;
    for ( auto const &ee : g_func_alterations_all )
    {
        for ( auto const &e : ee.second )
        {
            Method_Info const &mi = e.second;
            Function_Signature const &fs = mi.fsig;

            cout << Sv( g_intact.cbegin() + i, mi.iter_first_char );

            fs.Original_Function_Signature_Renamed(cout);

            cout << Sv( g_intact.cbegin() + mi.p_body->First(), g_intact.cbegin() + mi.p_body->Last() + 1u )
                 << endl
                 << endl;

            cout << mi.replacement_body.str() << endl
                 << endl
                 << endl;

            i = mi.p_body->Last() + 1u;
        }
    }

    cout << Sv( g_intact.cbegin() + i, g_intact.cend() );
}

void Print_Header(void)
{
    cout << "namespace Continuity_Methods {\n"
            "\n"
            "    typedef decltype(sizeof(char)) size_t;\n"
            "    typedef decltype((char*)0 - (char*)0) ptrdiff_t;\n"
            "\n"
            "    template<class T, T v>\n"
            "    struct integral_constant {\n"
            "        static constexpr T value = v;\n"
            "        using value_type = T;\n"
            "        using type = integral_constant; // using injected-class-name\n"
            "        constexpr operator value_type() const noexcept { return value; }\n"
            "        constexpr value_type operator()() const noexcept { return value; } // since c++14\n"
            "    };\n"
            "\n"
            "    using  true_type = integral_constant<bool,  true>;\n"
            "    using false_type = integral_constant<bool, false>;\n"
            "\n"
            "    template <class T, template <class...> class Test>\n"
            "    struct exists {\n"
            "        template<class U>\n"
            "        static true_type check(Test<U>*);\n"
            "\n"
            "        template<class U>\n"
            "        static false_type check(...);\n"
            "\n"
            "        static constexpr bool value = decltype(check<T>(0))::value;\n"
            "    };\n"
            "\n"
            "    template<typename T>\n"
            "    class reverse_iterator {\n"
            "    protected:\n"
            "\n"
            "        T *p;\n"
            "\n"
            "    public:\n"
            "\n"
            "        constexpr bool operator!=(reverse_iterator const &rhs) const\n"
            "        {\n"
            "            return p != rhs.p;\n"
            "        }\n"
            "\n"
            "        constexpr reverse_iterator(T *arg) : p(arg) {}\n"
            "\n"
            "        constexpr T &operator*(void) const\n"
            "        {\n"
            "            T *tmp = p;\n"
            "            return *--tmp;\n"
            "        }\n"
            "\n"
            "        constexpr reverse_iterator &operator++(void)\n"
            "        {\n"
            "            --p;\n"
            "            return *this;\n"
            "        }\n"
            "\n"
            "        constexpr reverse_iterator &operator--(void)\n"
            "        {\n"
            "            ++p;\n"
            "            return *this;\n"
            "        }\n"
            "\n"
            "        constexpr T &operator[](ptrdiff_t const arg) const\n"
            "        {\n"
            "            return p[static_cast<ptrdiff_t>(-arg - 1)];\n"
            "        }\n"
            "    };\n"
            "\n"
            "    template<typename T,size_t N>\n"
            "    struct reversion_wrapper {\n"
            "        \n"
            "        T (&t)[N];\n"
            "\n"
            "        constexpr reverse_iterator<T> begin(void)\n"
            "        {\n"
            "            return reverse_iterator<T>(t + N);\n"
            "        }\n"
            "\n"
            "        constexpr reverse_iterator<T> end(void)\n"
            "        {\n"
            "            return reverse_iterator<T>(t + 0u);\n"
            "        }\n"
            "\n"
            "    };\n"
            "\n"
            "    class reversed_order_t {};\n"
            "\n"
            "    template<typename T, size_t N>\n"
            "    constexpr reversion_wrapper<T,N> operator|(T (&arg)[N], reversed_order_t)\n"
            "    {\n"
            "        return { arg };\n"
            "    }\n"
            "\n"
            "    class intact_order_t {};\n"
            "\n"
            "    template<typename T, size_t N>\n"
            "    constexpr T (&operator|(T (&arg)[N], intact_order_t))[N]\n"
            "    {\n"
            "        return arg;\n"
            "    }\n"
            "\n"
            "      intact_order_t constexpr constructor_order;\n"
            "    reversed_order_t constexpr  destructor_order;\n"
            "}\n\n";
}

void my_terminate_handler(void)
{
    std::exception_ptr eptr = std::current_exception();

    try
    {
        if ( nullptr != eptr ) std::rethrow_exception(eptr);
    }
    catch(const std::exception& e)
    {
        clog << "=====================================================================================================" << endl;
        clog << "Exception: " << e.what() << endl;
        clog << "Total memory allocation: " << (g_total_allocation / 1024u / 1024u) << " megabytes" << endl;
        clog << "Milliseconds taken: " << GetTickCount() - g_timestamp_program_start << endl;
        clog << "=====================================================================================================" << endl;
    }

    std::abort();
}

#   if defined(_WIN32) || defined(_WIN64)
    extern "C" int _setmode(int fd,int mode);
    extern "C" int _fileno(std::FILE *stream);
#endif

int main(int const argc, char **const argv)
{
    g_timestamp_program_start = GetTickCount();

    std::set_terminate(my_terminate_handler);

    // The standard input stream, cin, is set to text mode
    // and so we need to set it to binary mode:
    bool cin_is_now_in_binary_mode = false;

#   if defined(_WIN32) || defined(_WIN64)
        cin_is_now_in_binary_mode = ( /* _O_TEXT */ 0x4000 == ::_setmode( ::_fileno(stdin), /* O_BINARY */ 0x8000) );
#   else
        cin_is_now_in_binary_mode = (nullptr != std::freopen(NULL, "rb", stdin));
#   endif

    if ( false == cin_is_now_in_binary_mode ) throw std::runtime_error("Could not set standard input to binary mode (it defaults to text mode)");

    cin >> std::noskipws;

    Print_Header();

    std::copy( istream_iterator<char>(cin), istream_iterator<char>(), back_inserter(g_intact) );

    clog << "Bytes: " << g_intact.size() << endl;
    clog << "Lines: " << Lines() << endl;

    std::replace(g_intact.begin(), g_intact.end(), '\0', ' ');  // null chars will screw up functions that parse null as end of string

    StringAlgorithms::replace_all(g_intact, "\r\n", " \n");

    StringAlgorithms::replace_all(g_intact, "\r", "\n");

    Replace_All_Preprocessor_Directives_With_Spaces();
    Replace_All_String_Literals_With_Spaces();

    //std::copy( g_intact.begin(), g_intact.end(), std::ostream_iterator<char>(clog) );
    
    g_curly_manager.Process();

    clog << "====================================== ALL PROCESSING DONE ===========================================" << endl;

    only_print_numbers = false;
    g_curly_manager.Print();

    //fifo_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    clog << "====================================== All scope names ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        clog << e.first << " [" << std::get<1u>(e.second) << "] ";

        for ( CurlyBracketManager::CurlyPair const *const  &my_curly_pair_pointer : std::get<0u>(e.second) )  // For classes, just one iteration. For namespaces, many iterations.
        {
            list< pair<size_t,size_t> > const my_list = GetOpenSpacesBetweenInnerCurlyBrackets(*my_curly_pair_pointer);

            for ( auto const my_pair : my_list )
            {
                clog << " Open[" << LineOf(my_pair.first)+1u << "-" << LineOf(my_pair.second)+1u << "]";
                Find_All_Usings_In_Open_Space(my_pair.first, my_pair.second, e.first + "::");
            }

            clog << endl;
        }
    }

    clog << "====================================== Now the pseudonyms (using X = Y;) ==============================================" << endl;
    for ( auto const &e : g_psuedonyms )
    {
        clog << e.first << " == " << e.second << endl;
    }

    clog << "====================================== Now the namespaces ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        if ( "namespace" != std::get<1u>(e.second) ) continue;

        clog << e.first << endl;
    }

    clog << "====================================== Now the classes that have base classes ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        if ( "struct" != std::get<1u>(e.second) && "class" != std::get<1u>(e.second) ) continue;

        list<string> const bases{ Get_All_Bases(e.first) };

        if ( bases.empty() ) continue;

        clog << e.first << " - Line#" << LineOf(std::get<0u>(e.second).front()->First())+1u << " to Line#" << LineOf(std::get<0u>(e.second).front()->Last())+1u
             << " | Bases = ";

        for ( auto const &b : bases )
        {
            clog << b << ", ";
        }

        clog << endl;
    }

    clog << "====================================== Classes containing methods marked 'continue' ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        if ( "struct" != std::get<1u>(e.second) && "class" != std::get<1u>(e.second) ) continue;

        for ( CurlyBracketManager::CurlyPair const *const  &p_curly_pair_pointer : std::get<0u>(e.second) )  // For classes, just one iteration. For namespaces, many iterations.
        {
            list< pair<size_t,size_t> > const my_list = GetOpenSpacesBetweenInnerCurlyBrackets(*p_curly_pair_pointer);

            for ( auto const my_pair : my_list )
            {
                if ( Find_All_Methods_Marked_Continue_In_Class(e.first, *p_curly_pair_pointer, my_pair.first, my_pair.second) )
                {
                    clog << e.first << endl << endl;

                    Print_Helper_Classes_For_Class(e.first);
                }
            }
        }
    }

    // In reverse order
    Print_Final_Output();
}
