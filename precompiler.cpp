/*
 * This program is one file of C++ code, and has no dependencies other
 * than the C++ standard library (minimum C++17).
 *
 * The purpose of this program is to implement the C++ proposal entitled
 * 'Continuity Methods'.
 *
 * This C++ source file is separated into 10 sections:
 * (Section  1) Build Modes - Debug, Release                              : Line #  22 - #  66 : (Total =   45 lines)
 * (Section  2) Override global 'new' and 'delete' for max speed          : Line #  67 - # 129 : (Total =   63 lines)
 * (Section  3) GradedOstream for selective control over output from clog : Line # 130 - # 270 : (Total =  141 lines)
 * (Section  4) Implementations of functions from Boost                   : Line # 271 - # 679 : (Total =  409 lines)
 * (Section  5) Implementation of container type : FIFO map               : Line # 680 - # 765 : (Total =   86 lines)
 * (Section  6) Include standard header files, and define global objects  : Line # 766 - # 837 : (Total =   72 lines)
 * (Section  7) regex_top_level_token_iterator                            : Line # 838 - #1101 : (Total =  264 lines)
 * (Section  8) Process keywords and identifiers                          : Line #1102 - #1211 : (Total =  110 lines)
 * (Section  9) Parse function signatures                                 : Line #1212 - #1789 : (Total =  578 lines)
 * (Section 10) Generate the auxillary code needed for Continuity Methods : Line #1790 - #1925 : (Total =  136 lines)
 * (Section 11) Parse all the class definitions in the input              : Line #1926 - #3454 : (Total = 1530 lines)
*/

// =====================================================================
// Section 1 of 11 : Build Modes - Debug, Release
// =====================================================================

#define FORBID_RECURSIVE_FUNCTION_CALLS

// The next macro is to detect the GNU C++ compiler (g++) on Linux
#if     defined(__GNUG__) &&  defined(__linux__)                       \
    && !defined(__llvm__) && !defined(__clang__)                       \
    && !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)   \
    && !defined(_WIN32) && !defined(_WIN64)                            \
    && !defined(__APPLE__)
#    define REAL_GNU_COMPILER
#endif

#if defined(_GLIBC_DEBUG) || defined(_GLIBC_DEBUG_PEDANTIC) || defined(_GLIBCXX_DEBUG) || defined(_GLIBCXX_DEBUG_PEDANTIC)
#    error "Please only set or unset NDEBUG. Don't manually set any of the following: _GLIBC_DEBUG _GLIBC_DEBUG_PEDANTIC _GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC"
#    include "Please_only_set_or_unset_NDEBUG____No_Such_Header_File"
#endif

#ifdef NDEBUG
     /* Release Mode */
#    ifdef REAL_GNU_COMPILER
#        include <list>         //         std::list
#        include <debug/list>   // __gnu_debug::list
#        include <type_traits>  // is_same_v
         static_assert( false == std::is_same_v< std::list<int>, __gnu_debug::list<int> >, "libstdc++ is using containers from __gnu_debug even though we're in Release Mode");
#        warning "libstdc++ is using containers from std"
#    endif
#else
     /* Debug Mode */
#    define _GLIBC_DEBUG
#    define _GLIBC_DEBUG_PEDANTIC
#    define _GLIBCXX_DEBUG
#    define _GLIBCXX_DEBUG_PEDANTIC
#    define _GLIBCXX_EXTERN_TEMPLATE 0
#    ifdef REAL_GNU_COMPILER
#        include <list>         //         std::list
#        include <debug/list>   // __gnu_debug::list
#        include <type_traits>  // is_same_v
         static_assert( std::is_same_v< std::list<int>, __gnu_debug::list<int> >, "libstdc++ is not using containers from __gnu_debug even though we're in Debug Mode");
#        warning "libstdc++ is using containers from __gnu_debug"
#    endif
#endif

// =================================================================
// Section 2 of 11 : Override global 'new' and 'delete' for max speed
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

// ===========================================================================
// Section 3 of 11 : GradedOstream for selective control over output from clog
// ===========================================================================

#include <ostream>   // ostream
#include <iostream>  // clog

enum eGrade : unsigned int {  // unsigned int is at least 16-Bit

/*
        00000001 Warnings for classes not found
        00000010 Classes containing a continuity marker
        00000100 Processing of continuity markings
        00001000 Summary banners at start and finish
        00010000 Replacement of preprocessor directives
        00100000 Replacement of string literals
        01000000 Location of every curly pair {}
        10000000 Intermediary Level 1 and Level 2 failures to find classes
       100000000 All scope names
      1000000000 All pseudonyms (typedef & using)
     10000000000 All namespaces
    100000000000 All classes that have base classes
   1000000000000 Open spaces inside namespace and class bodies
*/

    eGradeWarnings             = 0b00000001u,
    eGradeClassesWithContMarks = 0b00000010u,
    eGradeProcessContMarks     = 0b00000100u,

    eGradeBanners    = 0b00001000u,

    eGradePreproDirs = 0b00010000u,
    eGradeStringLits = 0b00100000u,
    eGradeCurlyPair  = 0b01000000u,
    eGradeFindFails  = 0b10000000u,

    eGradeAllScopes         =     0b100000000u,
    eGradeAllPseudonyms     =    0b1000000000u,
    eGradeAllNamespaces     =   0b10000000000u,
    eGradeAllDerivedClasses =  0b100000000000u,
    eGradeOpenSpaces        = 0b1000000000000u,
};

class Grade;

class GradedOstream {

    friend class Grade;

protected:

    bool is_on = false;
    std::ostream &os;
    unsigned grade_config = 0b1011u;
    unsigned most_recent_arg = 0u;

    void DetermineOnOrOff(void) noexcept
    {
        is_on = ( most_recent_arg == (most_recent_arg & grade_config) );
    }

    void SetConfig(unsigned const arg) noexcept
    {
        grade_config = arg;
        this->DetermineOnOrOff();
    }

    eGrade SetGrade(eGrade const arg) noexcept
    {
        eGrade const tmp = static_cast<eGrade>(most_recent_arg);

        most_recent_arg = arg;

        this->DetermineOnOrOff();

        return tmp;
    }

public:

    GradedOstream(std::ostream &arg_os) : os(arg_os) {}
    GradedOstream(std::ostream &arg_os, unsigned const arg_grade_config) : os(arg_os), grade_config(arg_grade_config) {}

    template<typename ArgType>
    GradedOstream &operator<<(ArgType const &arg)
    {
        static_assert( false == std::is_same_v<ArgType,eGrade>, "You must create a local object of type Grade" );

        if ( is_on ) os << arg;

        return *this;
    }

    GradedOstream &operator<<(std::ostream &(*funcptr)(std::ostream&))  // needed for "<< std::endl"
    {
        if ( is_on ) os << funcptr;

        return *this;
    }

    operator bool(void) const noexcept { return is_on; }

    GradedOstream(GradedOstream const & ) = delete;
    GradedOstream(GradedOstream       &&) = delete;
    GradedOstream &operator=(GradedOstream const & ) = delete;
    GradedOstream &operator=(GradedOstream       &&) = delete;
    GradedOstream const *operator&(void) const = delete;
    GradedOstream       *operator&(void)       = delete;
};

GradedOstream clogg(std::clog);

struct Grade {

protected:

    eGrade previous;
    GradedOstream &os;

public:

    Grade(GradedOstream &arg_os, eGrade const arg) : os(arg_os)
    {
        //std::cout << "Applying grade: " << (unsigned)arg << std::endl;
        previous = os.SetGrade(arg);
    }

    ~Grade(void)
    {
        //std::cout << "Restoring grade: " << (unsigned)previous << std::endl;
        os.SetGrade(previous);
    }

    Grade(Grade const & ) = delete;
    Grade(Grade       &&) = delete;
    Grade &operator=(Grade const & ) = delete;
    Grade &operator=(Grade       &&) = delete;
    Grade const *operator&(void) const = delete;
    Grade       *operator&(void)       = delete;
};

// ==========================================================================
// Section 4 of 11 : Implementations of functions from Boost
// ==========================================================================

#include <cassert>     // assert
#include <cstddef>     // size_t
#include <cstring>     // strcmp, strlen
#include <cctype>      // isalnum, isdigit
#include <string>      // string, to_string
#include <string_view> // string_view
#include <algorithm>   // all_of
#include <type_traits> // remove_reference_t, remove_cv_t, decay (all for C++17)
#include <regex>       // regex, match_results - REVISIT FIX - This is overkill!
#include <stdexcept>   // runtime_error - Maybe this shouldn't be used here

// The next two are just for the C++11 implementation of 'to_address'
#include <type_traits>  // enable_if, is_pointer
#include <utility>      // declval

std::string g_intact;

class StringAlgorithms {

    typedef std::size_t size_t;
    typedef std::string string;
    typedef std::string_view string_view;

private:

    static void trim_all(string &s)
    {
        if ( s.empty() ) return;

        size_t i;
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

        if ( s.empty() ) return;

        if ( std::isspace(static_cast<char unsigned>(s.front())) ) s.erase(            0u, 1u );

        if ( s.empty() ) return;

        if ( std::isspace(static_cast<char unsigned>(s.back() )) ) s.erase( s.size() - 1u, 1u );
    }

public:

    static void erase_all(string &s, string_view const sv)
    {
        if ( s.empty() || sv.empty() ) return;

        size_t i = 0u;
        while ( (s.size() != i) && (-1 != (i = s.find(sv, i))) )  // deliberate single '='
        {
            s.erase(i, sv.size());
        }
    }

    static void replace_all(string &s, string_view const sv_old, string_view const sv_new)
    {
        if ( s.empty() || sv_old.empty() ) return;

        size_t i = 0u;
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

    static bool IsIdChar(char const c) noexcept
    {
        return '_' == c || std::isalnum(static_cast<char unsigned>(c));
    }

private:

    static bool Is_Space_Necessary(string_view const s, size_t const i)
    {
        // Input string has already had all white
        // space reduced to one space, and it
        // neither ends nor begins with a space

        static char const *const double_syms[] = { "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "||", "&&", "==", "!=", ">=", "<=", "<<", ">>", ".*", "++", "--", "->", "::" };

        static char const *const tripple_syms[] = { "<<=", ">>=", "<=>", "->*" };

        assert( i < s.size() );
        assert( ' ' == s[i] );
        assert( (i + 1u) < s.size() );  // If this fails then 's' ends with a space

        if ( IsIdChar(s[i-1u]) && IsIdChar(s[i+1u]) )
        {
            return true;
        }

        for ( char const *const sss : tripple_syms )
        {
            char monkey[4u] = {};

            monkey[0u] = s[i - 1u];
            monkey[1u] = s[i + 1u];

            if ( (i + 2u) < s.size() )
            {
            monkey[2u] = s[i + 2u];

            if ( 0 == std::strcmp(sss,monkey) ) return true;
            }

            if ( i < 2u ) continue;

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

    static void Remove_Unnecessary_Spaces(string &s)
    {
        for ( size_t i = 0u;
                  i != s.size()
              && -1 != (i = s.find(' ',i));
              ++i )
        {
            if ( Is_Space_Necessary(s, i) ) continue;

            s.erase(i--,1u);
        }
    }

public:

    static void Minimise_Whitespace(string &s)
    {
        assert( false == s.empty() );
        trim_all(s);  // Removes leading whitespace, trailing whitespace, and reduces all other whitespace to one space ' '

        if ( s.empty() ) return;

        assert(   false == std::isspace( static_cast<char unsigned>(s.front()) )   );
        assert(   false == std::isspace( static_cast<char unsigned>(s.back() ) )   );

        Remove_Unnecessary_Spaces(s);
    }

    static bool Is_Entire_String_Valid_Identifier(string_view const sv)
    {
        assert( false == sv.empty() );

        if ( false == std::isdigit(static_cast<char unsigned>(sv[0u])) )  // First character can't be a digit
        {
            return std::all_of( sv.cbegin(), sv.cend(), [](char const c){ return IsIdChar(c); } );
        }
        else
        {
            return false;
        }
    }

    static size_t FindFirstId(string_view const haystack, string_view const needle)
    {
        assert( false == haystack.empty() );
        assert( false ==   needle.empty() );
        assert( Is_Entire_String_Valid_Identifier(needle) );

        size_t i = 0u;

        for (;; ++i)
        {
            i = haystack.find(needle, i);

            if ( -1 == i ) return -1;

            if (    (0u != i) && IsIdChar( haystack[i - 1u] )    ) continue;

            if ( ((i + needle.size()) < haystack.size()) && IsIdChar( haystack[i + needle.size()] ) ) continue;

            return i;
        }
    }

    static void EraseAllIds(string &s, string_view const id)
    {
        size_t i = 0u;

        while ( -1 != (i = FindFirstId(s,id)) )
        {
            s.erase(i,id.size());
        }
    }

    template<typename A, typename B>
    static bool starts_with(A const &a, B const &b)
    {
        using std::is_same_v;

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
    static bool ends_with(A const &a, B const &b)
    {
        using std::is_same_v;

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

private:

    template <typename T, typename std::enable_if<std::is_pointer<T>::value, bool>::type = true>
    static T cxx11_to_address(T arg)  // Pass pointers by value
    {
        return arg;
    }

    template <typename T,
              typename std::enable_if<false == std::is_pointer<T>::value, bool>::type = false,
              typename std::enable_if<std::is_pointer< decltype(std::declval<T>().operator->()) >::value, bool>::type = true>
    static decltype(std::declval<T>().operator->()) cxx11_to_address(T arg)  // Pass iterators by value
    {
        return arg.operator->();
    }

public:

    // The following function is to support C++17 which is lacking the
    // constructor for 'string_view' which takes two iterators. Also we want
    // to be able to mix and match different iterator types.
    template<typename A, typename B>
    static string_view Sv(A a, B b)  // Pass iterators by value
    {
        assert( cxx11_to_address(b) >= cxx11_to_address(a) );


        // The next line checks if the string_view is empty, and if it's
        // not empty it makes sure the last char is not a null char.
        // This is to try catch an error such as:
        //     char const brush[] = "brush";
        //     string_view( brush, brush + sizeof(brush) );
        assert( (cxx11_to_address(b) == cxx11_to_address(a)) || ('\0' != *(cxx11_to_address(b) - 1u)) );

        // Now we use the 'string_view::string_view' constructor which takes a pointer and an integer length.
        // Wise to use static_cast's here to be extra verbose as the class 'string_view' has several constructors.
        return std::string_view(
            static_cast<char const *>( cxx11_to_address(a) ),
            static_cast<std::string_view::size_type>(cxx11_to_address(b) - cxx11_to_address(a)) );
    }

    static size_t Find_Last_Double_Colon(std::string_view const s)  // REVISIT FIX - Don't use regex here, it's overkill
    {
        static std::regex const r("[:](?=[^\\<]*?(?:\\>|$))");  // matches a semi-colon so long as it's not enclosed in angle brackets

        std::match_results<std::string_view::const_reverse_iterator> my_match;

        if ( std::regex_search(s.crbegin(), s.crend(), my_match, r) )  // returns true if there is at least one match
        {
            size_t const index_of_second_colon_in_last_double_colon = &*(my_match[0u].first) - &s.front(); // REVISIT FIX - consider using my_match.position() here
            assert( ':' == s[index_of_second_colon_in_last_double_colon] );

            size_t const index_of_first_colon_in_last_double_colon  = index_of_second_colon_in_last_double_colon - 1u;
            if ( ':' != s[index_of_first_colon_in_last_double_colon] ) throw std::runtime_error("String should only contain a double-colon pair, but it contains a lone colon");

            //cout << "============ POSITION = " << index_of_first_colon_in_last_double_colon << " =================" << endl;

            return index_of_first_colon_in_last_double_colon;
        }

        return -1;
    }

    static void ThrowIfBadIndex(size_t const char_index)
    {
        if ( char_index >= g_intact.size() )
            throw std::runtime_error("Cannot access *(p + " + std::to_string(char_index) + ") inside type char[" + std::to_string(g_intact.size()) + "]");
    }

    static size_t LastChar(void)
    {
        assert(false == g_intact.empty());
        return g_intact.size() - 1u;
    }

    static size_t Lines(void) { return std::count( g_intact.begin(), g_intact.end(), '\n' ); }

    static size_t LineOf(size_t const char_index)
    {
        ThrowIfBadIndex(char_index);

        return std::count( g_intact.begin(), std::next(g_intact.begin(), char_index), '\n' );
    }

    static size_t StartLine(size_t char_index)
    {
        ThrowIfBadIndex(char_index);

        while ( (0u != char_index) && ('\n' != g_intact[char_index]) )
        {
            --char_index;
        }

        return char_index;
    }

    static size_t EndLine(size_t char_index)
    {
        ThrowIfBadIndex(char_index);

        while ( (LastChar() != char_index) && ('\n' != g_intact[char_index]) )
        {
            ++char_index;
        }

        return char_index;
    }
};

template<typename A, typename B>
static std::string_view Sv(A a, B b)  // Pass iterators by value
{
    return StringAlgorithms::Sv(a,b);
}

// ==========================================================================
// Section 5 of 11 : Implementation of container type : FIFO map
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
// Section 6 of 11 : Include standard header files, and define global objects
// =========================================================================

bool constexpr verbose = false;
bool constexpr print_all_scopes = false;

bool only_print_numbers; /* This gets set in main -- don't set it here */

#include <cstdlib>        // EXIT_FAILURE, abort
#include <cctype>         // isspace, isalpha, isdigit
#include <cstring>        // memset, memcpy
#include <cstdio>         // freopen, stdin
#include <iostream>       // cout, endl
#include <algorithm>      // copy, replace, count, all_of, any_of
#include <iterator>       // next, distance, back_inserter, istream_iterator, iterator_traits
#include <ios>            // ios::binary
#include <iomanip>        // noskipws
#include <stdexcept>      // out_of_range, runtime_error
#include <utility>        // pair<>
#include <string>         // string, to_string
#include <list>           // list
#include <set>            // set
#include <array>          // array
#include <tuple>          // tuple
#include <utility>        // pair, std::move()
#include <string_view>    // string_view
#include <regex>          // regex, regex_replace, smatch, match_results
#include <exception>      // exception
#include <chrono>         // duration_cast, milliseconds, steady_clock
#include <sstream>        // ostringstream
#include <atomic>         // atomic<>  - REVISIT - FIX - Not needed in single-threaded program

#ifdef FORBID_RECURSIVE_FUNCTION_CALLS
#    include <stack>          // stack (for iterative recursive function)
#endif

using std::cin;
using std::cout;
using std::endl;
using std::to_string;
using std::string_view;
using std::array;
using std::tuple;
using std::pair;

using std::string;
using std::list;
using std::set;

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
// Section 7 of 11 : regex_top_level_token_iterator
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
// Section 8 of 11 : Process keywords and identifiers
// ==========================================================================

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

            if ( (one_past_last < s.size()) && StringAlgorithms::IsIdChar(s[one_past_last]) )
            {
                //cout << "disregarding" << endl;
                continue;
            }

            if ( (0u != i) && StringAlgorithms::IsIdChar(s[i-1u]) )
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
// Section 9 of 11 : Parse function signatures
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

            while ( false == _name.empty() && false == StringAlgorithms::Is_Entire_String_Valid_Identifier(_name) )
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

                if ( (one_past_last < _original.size()) && StringAlgorithms::IsIdChar(_original[one_past_last]) )
                {
                    //cout << "disregarding AAA" << endl;
                    continue;
                }

                if (     (0u != location)       && StringAlgorithms::IsIdChar(_original[location-1u]) )
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

string TextBeforeOpenCurlyBracket(size_t const char_index)  // strips off the template part at the start, e.g. "template<class T>"
{
    StringAlgorithms::ThrowIfBadIndex(char_index);

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

    if ( retval.empty() ) return {};

    StringAlgorithms::Minimise_Whitespace(retval);
    StringAlgorithms::replace_all(retval, "::", "mOnKeY");
    StringAlgorithms::replace_all(retval, ":", " : ");
    StringAlgorithms::replace_all(retval, "mOnKeY", "::");

#if 1
    if ( StringAlgorithms::starts_with(retval, "template") ) return {};
#else
    //if ( retval.contains("allocator_traits") ) clogg << "1: ===================" << retval << "===================" << endl;
    retval = regex_replace(retval, regex("(template<.*>) (class|struct) (.*)"), "$2 $3");
    retval = regex_replace(retval, regex("\\s*,\\s*"), ",");
    //if ( retval.contains("allocator_traits") ) clogg << "2: ===================" << retval << "===================" << endl;
#endif

    return retval;
}

class CurlyBracketManager {
public:

    struct CurlyPair {
    protected:

        CurlyPair *_parent;
        pair<size_t,size_t> _indices;
        list<CurlyPair> _nested;

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

        list<CurlyPair> const &Nested(void) const noexcept { return _nested; }

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
        Grade g(clogg, eGradeCurlyPair);

        verbose && clogg << "- - - - - Print_CurlyPair( *(" << &cp << "), " << indentation << ") - - - - -" << endl;

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
                clogg << "    ";
            }

            auto &LineOf = StringAlgorithms::LineOf;

            clogg << cp.First() << " (Line #" << LineOf(cp.First())+1u << "), " << cp.Last() << " (Line #" << LineOf(cp.Last())+1u << ")";

            verbose && clogg << "  [Full line: " << TextBeforeOpenCurlyBracket(cp.First()) << "]";

            if ( false == only_print_numbers )
            {
                extern string GetNames(CurlyBracketManager::CurlyPair const &);

                //clogg << "    " << GetNames(cp);

                clogg << "  [" << GetNames(cp) << "]";
            }

            clogg << endl;
        }

        for ( CurlyPair const &e : cp.Nested() )
        {
            verbose && clogg << "    - - - About to recurse" << endl;
            Print_CurlyPair(e, indentation + 1u);
        }
    }

public:

    struct ParentError : std::exception { };

    void Process(void)
    {
        Grade g(clogg, eGradeBanners);

        verbose && clogg << "========= STARTING PROCESSING ===========" << endl;

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

        verbose && clogg << "========= ENDING PROCESSING ===========" << endl;
    }

    void Print(void) const
    {
        for ( CurlyPair const &e : _root_pair.Nested() )
        {
            verbose && clogg << "    - - - About to recurse" << endl;
            Print_CurlyPair(e);
        }
    }

    CurlyPair const &Root(void) const noexcept { return _root_pair; };

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
// Section 10 of 11 : Generate the auxillary code needed for Continuity Methods
// ==========================================================================

void Print_Helper_Classes_For_Class(string classname)
{
    cout << "// ==========================================================================\n"
            "// Helper classes for continuity methods within class " << classname << "\n"
            "// ==========================================================================\n\n";

    fifo_map<size_t, Method_Info> const &g_func_alterations = g_func_alterations_all.at(classname);

    StringAlgorithms::replace_all(classname, "::", "_scope_");

    cout << "namespace Continuity_Methods { namespace Helpers { namespace " << classname << " {\n\n";

    cout << "namespace Testers {\n\n";
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
    "\n"
    "        // All methods have one extra\n"
    "        // parameter for 'this' as 'void*'\n";

    for ( auto const &e : g_func_alterations )
    {
        string_view const name = e.second.fsig.Name();

        cout << "        ";

        e.second.fsig.Signature_Of_Replacement_Function____With_Void_Pointer_This(cout);

        cout << " override\n" <<
                "        {\n"
                "            Base *const p_base_x7c1 = static_cast<Base*>(static_cast<Derived*>(arg_this));\n"
                "\n"
                "            if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::" << classname << "::Testers::" << name << "____WITHOUT_CONTINUITY>::value )\n"
                "            {\n"
                "                return p_base_x7c1->Base::";

        e.second.fsig.Invocation____WITHOUT_CONTINUITY(cout);

        cout << ";\n"
                "            }\n"
                "            else if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::" << classname << "::Testers::" << name << ">::value )\n"
                "            {\n"
                "                return p_base_x7c1->Base::";

        e.second.fsig.Invocation(cout);

        cout << ";\n"
                "            }\n"
                "            else\n"
                "            {\n"
                "                return decltype(static_cast<Derived*>(arg_this)->";

        e.second.fsig.Invocation(cout);

        cout << ")();\n"
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
// Section 11 of 11 : Parse all the class definitions in the input
// =============================================================

void Replace_All_String_Literals_With_Spaces(bool undo = false)
{
    Grade g(clogg, eGradeStringLits);

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

                clogg << "Replacing string literal at index " << i << " : [" << strlits[i] << "]" << endl;

                std::memset(&g_intact[i], ' ', k - i);

                i = k;  // i will be incremented on the next 'for' iterator
                break;
            }

            if ( 1u == (count % 2u) ) throw runtime_error("Unmatched double-quote in translation unit");
        }
    }

    clogg << "String Literal Replacements:\n"
            "============================\n";

    for ( auto const &e : strlits )
    {
        clogg << "Index " << e.first << ", Len = " << e.second.size() << ", [" << e.second << "]" << endl;
    }
}

void Replace_All_Preprocessor_Directives_With_Spaces(bool undo = false)
{
    Grade g(clogg, eGradePreproDirs);

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

    clogg << "Preprocessor Replacements:\n"
            "==========================\n";

    for ( auto const &e : directives )
    {
        clogg << "Index " << e.first << ", Len = " << e.second.size() << ", [" << e.second << "]" << endl;
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

list< array<string,3u> > Parse_Bases_Of_Class(string str)
{
    StringAlgorithms::replace_all(str, "virtual::"  , "virtual ::");
    StringAlgorithms::replace_all(str, "public::"   , "public ::");
    StringAlgorithms::replace_all(str, "protected::", "protected ::");
    StringAlgorithms::replace_all(str, "private::"  , "private ::");

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

        //clogg << "[ADD RECORD : " << std::get<0u>(tmp) << " : " << std::get<1u>(tmp) << " : " << std::get<2u>(tmp) << "]";
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

    if ( StringAlgorithms::starts_with(intro, "namespace") )
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

        return { "namespace", str, list< array<string,3u> >() };
    }

    if (   !(StringAlgorithms::starts_with(intro,"class") || StringAlgorithms::starts_with(intro,"struct"))   ) return {};

    StringAlgorithms::EraseAllIds( intro, "final" );

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

    if ( ++iter == sregex_top_level_token_iterator() ) return { "class", str, list< array<string,3u> >() };  // This bring us to the sole colon
    if ( ++iter == sregex_top_level_token_iterator() ) return { "class", str, list< array<string,3u> >() };  // This brings it to the first word after the colon (e.g. virtual)

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
        for ( CurlyBracketManager::CurlyPair const *p = &cp; (p = p->Parent()); /* no post-processing */ )  // will throw exception when root pair is reached
        {
            //clogg << "Iteration No. " << iteration << endl;

            verbose && clogg << " / / / / / / About to call Word_For_Curly_Pair(" << p->First() << ", " << p->Last() << ")" << endl;
            string const tmp = std::get<1u>( Intro_For_Curly_Pair(*p) );
            verbose && clogg << " / / / / / / Finished calling Word_For_Curly_Pair" << endl;

            if ( tmp.empty() ) continue;

            retval.insert(0u, tmp + "::");
        }
    }
    catch(CurlyBracketManager::ParentError const &)
    {
        //clogg << "********** ParentError ************";
    }

    retval.insert(0u, "::");

    //fifo_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    std::get<0u>( g_scope_names[retval] ).push_back(&cp);  // Note that these are pointers

    std::get<1u>( g_scope_names[retval] ) = std::get<0u>(tmppair);

    std::get<2u>( g_scope_names[retval] ) = std::get<2u>(tmppair);

    return retval;
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

    if ( (str.size() < (2*seplen + 1u)) || (false == StringAlgorithms::starts_with(str,separator)) || (false == StringAlgorithms::ends_with(str,separator)) )  // minimum = "::A::"
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

void Expose_Pseudonym(string &arg_prefix, string &arg_classname)
{
    bool constexpr debugthisfunc = false;

    debugthisfunc && clogg << "Entered function Expose_Pseudonym ========= "
                           << "Inputs = '" << arg_prefix << "' + '" << arg_classname << "', ";

    string const what_we_looked_up{
        StringAlgorithms::starts_with(arg_classname, "::")
        ? arg_classname
        : arg_prefix + arg_classname };

    try
    {
        arg_classname = g_psuedonyms.at(what_we_looked_up);  // If this throws then the next line isn't executed
        arg_prefix.clear();
    }
    catch(std::out_of_range const &e) {}

    size_t const index_of_last_colon = StringAlgorithms::Find_Last_Double_Colon(arg_classname);
    if ( -1 != index_of_last_colon )  /* Maybe it's like this:   Class MyClass : public ::SomeClass {}; */
    {
        arg_prefix    += arg_classname.substr(0u, index_of_last_colon + 2u);
        arg_classname  = arg_classname.substr(index_of_last_colon + 2u);
    }

    debugthisfunc && clogg << "Outputs = '" << arg_prefix << "' + '" << arg_classname << "', " << endl;

    assert( false == (arg_prefix.empty() && arg_classname.empty()) );
};


string Find_Class_Relative_To_Scope(string &prefix, string classname)
{
    assert( false == (prefix.empty() && classname.empty()) );

    decltype(g_scope_names)::mapped_type const *p = nullptr;

    if ( StringAlgorithms::starts_with(classname,"::") )  // If it's an absolute class name rather than relative
    {
        g_scope_names.at(classname);  // just to see if it throws
        return classname;
    }

    if ( -1 != prefix.find("<") || -1 != classname.find("<") ) return {};  // Don't tolerate any templates

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
            clogg << tmp;
            std::cerr << tmp;
            std::abort();
        }

        Expose_Pseudonym(prefix, classname);

        full_name  = prefix;
        full_name += classname;

        try
        {
            p = &( g_scope_names.at(full_name) );
        }
        catch (std::out_of_range const &)
        {
            Grade g(clogg, eGradeFindFails);

            clogg << " - - - FIRST FAILED - - - Prefix='" << prefix << "', Classname='" << classname << "' - Fullname='" << full_name << "'" << endl;

            string class_name_without_template_specialisation = regex_replace( string(classname), regex("<.*>"), "");  // REVISIT FIX -- gratuitous memory allocations

            if ( class_name_without_template_specialisation != classname )
            {
                string duplicate_original_full_name{ full_name };  // not const because we std::move() from it later

                Expose_Pseudonym(prefix, class_name_without_template_specialisation);

                full_name  = prefix;
                full_name += class_name_without_template_specialisation;

                try
                {
                     p = &( g_scope_names.at(full_name) );
                }
                catch (std::out_of_range const &)
                {
                    Grade g(clogg, eGradeFindFails);

                    clogg << " - - - SECOND FAILED - - - Prefix='" << prefix << "', Classname='" << class_name_without_template_specialisation << "' - Fullname='" << full_name << "'" << endl;

                    full_name = std::move(duplicate_original_full_name);
                }
            }
        }

        if ( nullptr != p )
        {
            //clogg << "Success: found '" << string(classname) << "') as ('" << full_name << "')";
            break;  // If we already have found the class then no need to keep searching
        }

        // Last resort: Change the prefix from "::std::__cxx11" into "::std", so that "::std::__cxx11::locale::facet" becomes "::std::locale::facet"
        if ( false == Strip_Last_Scope(prefix) ) break;
    }

    if ( nullptr == p )
    {
        //throw std::out_of_range("Encountered a class name that hasn't been defined ('" + string(classname) + "') referenced inside ('" + string(intact_prefix) + "')");

        Grade g(clogg, eGradeWarnings);
        clogg << "WARNING: Cannot find base class '" + string(classname) + "' referenced inside '" + string(intact_prefix) + "'" << endl;

        return {};
    }

    return full_name;
}

#ifdef FORBID_RECURSIVE_FUNCTION_CALLS

list<string> Get_All_Bases(string_view const arg)
{
    size_t const beginning_index_of_last_colon = StringAlgorithms::Find_Last_Double_Colon(arg);

    if ( -1 == beginning_index_of_last_colon ) throw runtime_error("There's no double-colon in the argument to Get_All_Bases");

    string_view const prefix    = arg.substr(0u, beginning_index_of_last_colon + 2u);
    string_view const classname = arg.substr(beginning_index_of_last_colon +  2u);

    set<string> already_recorded;

    list<string> retval;

    //Get_All_Bases_Recursively(string(prefix), string(classname), already_recorded, false, retval);

// =======================================================================================================
// =======================================================================================================
// =======================================================================================================
// =======================================================================================================
    bool constexpr debugthisfunc = false;

    struct StackFrame {
                                      string        prefix      ;
                                      string        classname   ;
                                        bool        is_virtual  ;
        decltype(g_scope_names)::mapped_type const *p           ;
                                      string        full_name   ;
                                        bool        is_new_entry;
    list< array<string,3u> >::const_iterator        it          ;
                    list< array<string,3u> > const *p_list_array;
                                      string        base_name   ;
    };

    std::stack<StackFrame> my_stack;

    my_stack.push(StackFrame{string(prefix),string(classname),false /* is_virtual */ });

    string last_recursive_return_value;

    for (bool should_pop = false; false == my_stack.empty(); should_pop && (my_stack.pop(),true) )
    {
    bool outer_continue = false;

                                      string        &prefix       = my_stack.top().prefix;
                                      string        &classname    = my_stack.top().classname;
                                        bool        &is_virtual   = my_stack.top().is_virtual;
        decltype(g_scope_names)::mapped_type const *&p            = my_stack.top().p;
                                      string        &full_name    = my_stack.top().full_name;
                                        bool        &is_new_entry = my_stack.top().is_new_entry;
    list< array<string,3u> >::const_iterator        &it           = my_stack.top().it;
                    list< array<string,3u> > const *&p_list_array = my_stack.top().p_list_array;
                                      string        &base_name    = my_stack.top().base_name;

    size_t index_of_last_colon = -1;  // only here because must define before the 'goto'

    if ( should_pop )
    {
        should_pop = false;
        goto Label__Jump_To_Here_After_Popping;
    }

    debugthisfunc && clogg << "231 : Recursive_Print_All_Bases_PROPER entered with arguments '" << prefix << "' + '" << classname << "'" << endl;

    if ( prefix.empty() ) std::abort();

    p = nullptr;

    Expose_Pseudonym(prefix, classname);

    full_name = Find_Class_Relative_To_Scope(prefix, classname); // This will throw if class is unknown

    debugthisfunc && clogg << "233 : full_name = '" << full_name << "' was gotten from '" << prefix << "' + '" << classname << "'" << endl;

    if ( full_name.empty() )  // not a fatal error if we can't find a base class that's a template class
    {
        debugthisfunc && clogg << "234 : Recursive is returning false because full_name is empty" << endl;

        should_pop = true;
        last_recursive_return_value = {};
        continue;
    }

    is_new_entry = already_recorded.insert(full_name).second;  // set::insert returns a pair, the second is a bool saying if it's a new entry

    if ( (false == is_new_entry) && is_virtual ) // REVISIT - FIX - all virtuals are common, all non-virtuals are unique
    {
        debugthisfunc && clogg << "235 : Recursive is returning false because it's NOT a new entry and it's virtual" << endl;
        //return {};  // if it's not a new entry and if it's virtual - REVISIT FIX all virtuals are common, and all non-virtuals are unique
        should_pop = true;
        last_recursive_return_value = {};
        continue;
    }

    p_list_array = &std::get<2u>( g_scope_names.at(full_name) );

    for ( it = p_list_array->cbegin(); it != p_list_array->cend(); ++it )
    {
        base_name = std::get<2u>(*it);  // Note that this is a simple name, i.e. "Laser" instead of "::MyNamespace::Laser"
        debugthisfunc && clogg << "236 : full_name '" << full_name << "' has base: '" << base_name << "'" << endl;

        if constexpr ( verbose )
        {
            clogg << " [ALREADY_SEEN=";
            for ( auto const &e_already : already_recorded ) clogg << e_already << ", ";
            clogg << "] ";
            clogg << " [[[VIRTUAL=" << (("virtual" == std::get<0u>(*it)) ? "true]]]" : "false]]] ") << endl;
        }

        index_of_last_colon = StringAlgorithms::Find_Last_Double_Colon(classname);
        if ( -1 != index_of_last_colon && (false == StringAlgorithms::starts_with(classname, "::")) )  /* Maybe it's like this:   Class MyClass : public ::SomeClass {}; */
        {
            debugthisfunc && clogg << "237 : Changing '" << prefix << "' and '" << classname << "' to '";
            // This deals with the case of a class inheriting from 'std::runtime_error', which inherits from 'exception' instead of 'std::exception'
            prefix += classname.substr(0u, index_of_last_colon);
            prefix += "::";

            debugthisfunc && clogg << prefix << "' and '" << classname << endl;
        }
        else if ( StringAlgorithms::starts_with(classname, "::") )
        {
            debugthisfunc && clogg << "238 : Wiping out the prefix because classname is '" << classname << "'" << endl;
            prefix.clear();  // In case the base class begins with two colons: "class MyClass : public ::SomeClass {};"
        }

        debugthisfunc && clogg << "============= 239 : Out to call recursively" << endl;
        //Get_All_Bases_Recursively(prefix, base_name, already_recorded, "virtual" == std::get<0u>(*it), retval);
        my_stack.push(StackFrame{prefix,base_name,"virtual" == std::get<0u>(*it)});
        outer_continue = true;
        break;

Label__Jump_To_Here_After_Popping:

        if ( false == last_recursive_return_value.empty() )
        {
            debugthisfunc && clogg << "240 : Adding base class to list of base classes retval.push_back(\" " << last_recursive_return_value << "\")" << endl;
            retval.push_back( last_recursive_return_value );
        }
        else
        {
            debugthisfunc && clogg << "241 : The return value from the recursive call was an empty string" << endl;
        }
    }

    if ( outer_continue ) continue;

    should_pop = true;
    last_recursive_return_value = full_name;
    }
// =======================================================================================================
// =======================================================================================================
// =======================================================================================================

    return retval;
}

#else

string Get_All_Bases_Recursively(string prefix, string classname, set<string> &already_recorded, bool is_virtual, list<string> &retval)
{
    bool constexpr debugthisfunc = false;

    debugthisfunc && clogg << "231 : Recursive_Print_All_Bases_PROPER entered with arguments '" << prefix << "' + '" << classname << "'" << endl;

    if ( prefix.empty() ) std::abort();

    decltype(g_scope_names)::mapped_type const *p = nullptr;

    Expose_Pseudonym(prefix, classname);

    string const full_name = Find_Class_Relative_To_Scope(prefix, classname); // This will throw if class is unknown

    debugthisfunc && clogg << "233 : full_name = '" << full_name << "' was gotten from '" << prefix << "' + '" << classname << "'" << endl;

    if ( full_name.empty() )
    {
        debugthisfunc && clogg << "234 : Recursive is returning false because full_name is empty" << endl;
        return {};  // not a fatal error if we can't find a base class that's a template class
    }

    bool const is_new_entry = already_recorded.insert(full_name).second;  // set::insert returns a pair, the second is a bool saying if it's a new entry

    if ( (false == is_new_entry) && is_virtual )
    {
        debugthisfunc && clogg << "235 : Recursive is returning false because it's NOT a new entry and it's virtual" << endl;
        return {};  // if it's not a new entry and if it's virtual - REVISIT FIX all virtuals are common, and all non-virtuals are unique
    }

    for ( auto const &e : std::get<2u>( g_scope_names.at(full_name) ) )
    {
        string const &base_name = std::get<2u>(e);  // Note that this is a simple name, i.e. "Laser" instead of "::MyNamespace::Laser"
        debugthisfunc && clogg << "236 : full_name '" << full_name << "' has base: '" << base_name << "'" << endl;

        if constexpr ( verbose )
        {
            clogg << " [ALREADY_SEEN=";
            for ( auto const &e_already : already_recorded ) clogg << e_already << ", ";
            clogg << "] ";
            clogg << " [[[VIRTUAL=" << (("virtual" == std::get<0u>(e)) ? "true]]]" : "false]]] ") << endl;
        }

        size_t const index_of_last_colon = StringAlgorithms::Find_Last_Double_Colon(classname);
        if ( -1 != index_of_last_colon && (false == StringAlgorithms::starts_with(classname, "::")) )  /* Maybe it's like this:   Class MyClass : public ::SomeClass {}; */
        {
            debugthisfunc && clogg << "237 : Changing '" << prefix << "' and '" << classname << "' to '";
            // This deals with the case of a class inheriting from 'std::runtime_error', which inherits from 'exception' instead of 'std::exception'
            prefix += classname.substr(0u, index_of_last_colon);
            prefix += "::";

            debugthisfunc && clogg << prefix << "' and '" << classname << endl;
        }
        else if ( StringAlgorithms::starts_with(classname, "::") )
        {
            debugthisfunc && clogg << "238 : Wiping out the prefix because classname is '" << classname << "'" << endl;
            prefix.clear();  // In case the base class begins with two colons: "class MyClass : public ::SomeClass {};"
        }

        debugthisfunc && clogg << "============= 239 : Out to call recursively" << endl;
        string str{ Get_All_Bases_Recursively(prefix, base_name, already_recorded, "virtual" == std::get<0u>(e), retval) };

        if ( false == str.empty() )
        {
            debugthisfunc && clogg << "240 : Adding base class to list of base classes retval.push_back(\" " << str << "\")" << endl;
            retval.push_back( std::move(str) );
        }
        else
        {
            debugthisfunc && clogg << "241 : The return value from the recursive call was an empty string" << endl;
        }
    }

    return full_name;
}

list<string> Get_All_Bases(string_view arg)
{
    size_t const index_of_last_colon = StringAlgorithms::Find_Last_Double_Colon(arg);

    if ( -1 == index_of_last_colon ) throw runtime_error("There's no double-colon in the argument to Get_All_Bases");

    string_view const prefix    = arg.substr(0u, index_of_last_colon + 2u);
    string_view const classname = arg.substr(index_of_last_colon +  2u);

    set<string> already_recorded;

    list<string> retval;

    Get_All_Bases_Recursively(string(prefix), string(classname), already_recorded, false, retval);

    return retval;
}

#endif

list< pair<size_t,size_t> > GetOpenSpacesBetweenInnerCurlyBrackets(CurlyBracketManager::CurlyPair const &cp)
{
    size_t first, last;

    if ( &g_curly_manager.Root() == &cp )
    {
        first = -1;               // This will have 1    added    to  it to make it sane
        last  = g_intact.size();  // This will have 1 subtracted from it to make it sane
    }
    else
    {
        assert( cp.Last() > cp.First() );

        first = cp.First();
        last  = cp.Last ();
    }

    // Check 1: To see if there's nothing between the curly brackets
    if ( last == (first + 1u) ) return {};   // e.g. if we have "class Monkey {};"

    // Check 2: To see if there's no inner curly brackets
    if ( cp.Nested().empty() )
    {
        return { { first + 1u, last - 1u }  }; // e.g. if we have "class Monkey { int i; };"
    }

    // If control reaches here, we have nested curly brackets

    list< pair<size_t,size_t> > retval;

    list<CurlyBracketManager::CurlyPair>::const_iterator iter = cp.Nested().cbegin();

    size_t begin_from = first + 1u;

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
            clogg << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::cerr << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::abort();
        }
    }

    assert( last >= begin_from );

    if ( last > begin_from ) // must rule out "namespace Dog { namespace Cat { int i; }}"
    {
        retval.push_back( { begin_from, last - 1u } );
    }

    return retval;
}

bool Query_Should_Disregard_Type(string_view const sv)
{
    assert( false == sv.empty() );

    string str(sv);

    StringAlgorithms::Minimise_Whitespace(str);

    assert( false == str.empty() );

    // Disregard types beginning with double underscores like __uint128_t
    if ( (sv.size() >= 2u) && ('_' == sv[0u]) && ('_' == sv[1u]) ) return true;

    if ( (str.size() >= 9u) && ("decltype(" == str.substr(0u,9u)) )
    {
        Grade g(clogg, eGradeWarnings);
        clogg << "WARNING: When parsing 'typedef' or 'using' declarations, cannot process a 'decltype' expression: " << str << endl;
        return true;
    }

    Find_And_Erase_All_Keywords(str);

    if ( str.empty() ) return true;

    StringAlgorithms::Minimise_Whitespace(str);

    return str.empty();
}

void Find_All_Usings_In_Open_Space(size_t const first, size_t const last, string scope_name)
{
    assert( last >= first );  // It's okay for them to be equal if we have "{ }"

    assert( StringAlgorithms::ends_with(scope_name,"::") );

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

        if ( Query_Should_Disregard_Type(original) ) continue;

        try
        {
            string full_name_of_original = Find_Class_Relative_To_Scope(scope_name, original);  // might throw out_of_range
            //cout << "Old = " << original << ", New = " << impersonator << endl;
            if ( full_name_of_original.empty() ) throw std::out_of_range("");
            g_psuedonyms[scope_name + impersonator] = full_name_of_original;
        }
        catch (std::out_of_range const &e)
        {
            Grade g(clogg, eGradeWarnings);
            clogg << "WARNING: When parsing 'using'   declaration, cannot find class '" << original << "' relative to scope '" << scope_name << "'" << endl;
            return;
        }
    }

    r = "typedef\\s+(.+?)\\s+(.*)\\s*;";

    for(std::sregex_iterator i  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             i != std::sregex_iterator();
                             ++i )
    {
        //clogg << "Iterating in typedef loop" << endl;

        string impersonator = regex_replace( i->str(), r, "$2" );
        string original     = regex_replace( i->str(), r, "$1" );

        impersonator = regex_replace(impersonator, regex("typename\\s*"), "");
        impersonator = regex_replace(impersonator, regex("template\\s*"), "");
        original = regex_replace(original, regex("typename\\s*"), "");
        original = regex_replace(original, regex("template\\s*"), "");

        StringAlgorithms::Minimise_Whitespace(impersonator);
        StringAlgorithms::Minimise_Whitespace(original    );

        if ( Query_Should_Disregard_Type(original) ) continue;

        try
        {
            string full_name_of_original = Find_Class_Relative_To_Scope(scope_name, original);  // might throw out_of_range
            //cout << "Old = " << original << ", New = " << impersonator << endl;
            if ( full_name_of_original.empty() ) throw std::out_of_range("");
            g_psuedonyms[scope_name + impersonator] = full_name_of_original;
        }
        catch (std::out_of_range const &e)
        {
            Grade g(clogg, eGradeWarnings);
            clogg << "WARNING: When parsing 'typedef' declaration, cannot find class '" << original << "' relative to scope '" << scope_name << "'" << endl;
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

                //clogg << "\nPutting in stream [" << buf << "]" << endl;

                os << buf << endl;

                PrintIndentation();

                p = q + 1u;
            }
        }

        //clogg << "\nPutting in stream [" << p << "]" << endl;
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
    Grade g(clogg, eGradeProcessContMarks);

    using Indent   = IndentedOstream::Indent;
    using Unindent = IndentedOstream::Unindent;

    assert( last >= first );  // It's okay for them to be equal if we have "{ }"

    bool retval = false;

    regex r("([A-z_][A-z_0-9]*)\\s+([A-z_][A-z_0-9]*)\\s*\\((.*)\\)\\s*.*(continue)\\s*(;|)");

    for(std::sregex_iterator iter  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             iter != std::sregex_iterator();
                             ++iter )
    {
        bool is_destructor_order = false;

        clogg << "At least one continue method found" << endl;

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

        size_t longest_name_colons = svclass.size();

        string derived_replaced(svclass);
        StringAlgorithms::replace_all(derived_replaced, "::", "_scope_");

        size_t longest_name_scopes = derived_replaced.size();

        for ( auto &e : bases )
        {
            if ( e.size() > longest_name_colons ) longest_name_colons = e.size();

            StringAlgorithms::replace_all(e, "::", "_scope_");

            if ( e.size() > longest_name_scopes ) longest_name_scopes = e.size();
        }

        IndentedOstream s( g_func_alterations[index].replacement_body, Indentation_For_CurlyPair(cp_body) );

        s << "\n";

        g_func_alterations[index].fsig.Signature_Of_Replacement_Function(s);

        s << "\n{";

        s << Indent();

        s << "\nusing namespace Continuity_Methods::Helpers::" << derived_replaced << ";\n\n";

        s << "using MethodInvokerT = MethodInvoker<" << svclass << ">;\n\n";

        for ( auto &e : bases )
        {
            StringAlgorithms::replace_all(e, "_scope_", "::");

            s << "static MethodInvokerT::MI< " << e;

            assert( longest_name_colons >= e.size() );

            for ( size_t i = 0u; i != (longest_name_colons - e.size()); ++i ) s << ' ';

            s << " > mi_";

            StringAlgorithms::replace_all(e, "::", "_scope_");

            s << e;

            assert( longest_name_scopes >= e.size() );

            for ( size_t i = 0u; i != (longest_name_scopes - e.size()); ++i ) s << ' ';

            s << ";\n";
        }

        s << "static MethodInvokerT::MI< " << svclass;

        assert( longest_name_colons >= svclass.size() );
        for ( size_t i = 0u; i != (longest_name_colons - svclass.size()); ++i ) s << ' ';

        s << " > mi_" << derived_replaced;

        assert( longest_name_scopes >= derived_replaced.size() );
        for ( size_t i = 0u; i != (longest_name_scopes - derived_replaced.size()); ++i ) s << ' ';

        s << ";\n";

        s << "\nInvoker methods[" << bases.size() + 1u << "u] = {";
        s << Indent();
        for ( auto &e : bases )
        {
            s << "\nInvoker(mi_" << e;

            assert( longest_name_scopes >= e.size() );
            for ( size_t i = 0u; i != (longest_name_scopes - e.size()); ++i ) s << ' ';

            s << ", " << "this),";
        }

        s << "\nInvoker(mi_" << derived_replaced;

        assert( longest_name_scopes >= derived_replaced.size() );
        for ( size_t i = 0u; i != (longest_name_scopes - derived_replaced.size()); ++i ) s << ' ';

        s << ", " << "this),";

        s << Unindent();

        s << "\n};\n\n";

        s << "for ( size_t i = " << ( is_destructor_order ? bases.size() : 0u  )
          << "u; /* true */; " << ( is_destructor_order ? "--" : "++" ) << "i )\n"
             "{\n"
             "    if ( " << ( is_destructor_order ? 0u : bases.size() ) << "u == i ) return methods[" << ( is_destructor_order ? 0u : bases.size() ) << "u].";

        g_func_alterations[index].fsig.Invocation(s);

        s << ";\n"
          << "\n"
             "    methods[i].";

        g_func_alterations[index].fsig.Invocation(s);

        s << ";\n"
             "}";

        s << Unindent();

        s << "\n}";
    }

    return retval;
}

#if 0
void Instantiate_Scope_By_Scope_Where_Necessary(string_view str)
{
    assert( false == str.empty() );

    char const separator[] = "::";

    size_t constexpr seplen = sizeof(separator) - 1u;

    if ( separator == str ) return; // If the input is "::"

    if ( StringAlgorithms::ends_with(str,separator) ) str.remove_suffix(seplen);  // Turn "::A::B::" into "::A::B"

    if ( (str.size() < (seplen + 1u)) || (false == StringAlgorithms::starts_with(str,separator)) || StringAlgorithms::ends_with(str,separator) )  // minimum = "::A"
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

        clogg << "Level " << i << ": " << Sv(str.cbegin(), iter->second);

        if ( -1 != Sv(iter->first,iter->second).find('<') )  // check if right-most is a template class
        {
            clogg << "    <---- template class";
        }

        clogg << endl;
    }
}
#endif

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

            /* REVISIT - FIX - Add code here to put in whitespace between function signature and open curly bracket */

            cout << Sv( g_intact.cbegin() + mi.p_body->First(), g_intact.cbegin() + mi.p_body->Last() + 1u )
                 << endl;

            cout << mi.replacement_body.str();

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
            "    typedef decltype((char*)nullptr - (char*)nullptr) ptrdiff_t;\n"
            "\n"
            "    struct  true_type { static constexpr bool value = true ; };\n"
            "    struct false_type { static constexpr bool value = false; };\n"
            "\n"
            "    template <class T, template <class...> class Test>\n"
            "    struct exists {\n"
            "\n"
            "        template<class U>\n"
            "        static true_type check(Test<U>*);\n"
            "\n"
            "        template<class U>\n"
            "        static false_type check(...);\n"
            "\n"
            "        static constexpr bool value = decltype(check<T>(0))::value;\n"
            "    };\n"
            "\n"
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
        Grade g(clogg, eGradeBanners);
        clogg << "=====================================================================================================" << endl;
        clogg << "Exception: " << e.what() << endl;
        clogg << "Total memory allocation: " << (g_total_allocation / 1024u / 1024u) << " megabytes" << endl;
        clogg << "Milliseconds taken: " << GetTickCount() - g_timestamp_program_start << endl;
        clogg << "=====================================================================================================" << endl;
    }

    std::abort();
}

/* =====================================================================
   =====================================================================
   =====================================================================
     MS-Windows opens 'stdin' in text mode, and so we use the function
     '_setmode' or 'setmode' to change it to binary mode. The Microsoft
     compiler calls it '_setmode', while the Embarcadero (formerly known
     as Borland) compiler calls it 'setmode'. */

#include <cstdio>  // stdin freopen (for Linux), FILE (for MS-Windows)

#if defined(_WIN32) || defined(_WIN64)
     extern "C" int _fileno(std::FILE *);
#    ifdef __BORLANDC__
         extern "C" int setmode(int,int);
         inline int _setmode(int const fd, int const mode)
         {
             return setmode(fd,mode);
         }
#    else
         extern "C" int _setmode(int,int);
#    endif  // ifdef __BORLANDC__
#endif

template<class T>
static void SetCinToBinary(char const *const runtime_error_text)
{

#if defined(_WIN32) || defined(_WIN64)
    bool const b = (0x4000 == ::_setmode( ::_fileno(stdin), /* O_BINARY */ 0x8000));
#else
    bool const b = (nullptr != std::freopen(NULL, "rb", stdin));
#endif

    if ( false == b ) throw T(runtime_error_text);
}
/* ==================================================================
   ==================================================================
   ================================================================== */

int main(int const argc, char **const argv)
{
    g_timestamp_program_start = GetTickCount();

    std::set_terminate(my_terminate_handler);

    Grade g(clogg, eGradeBanners);

    // The standard input stream, cin, is set to text mode,
    // and so we need to set it to binary mode:
    SetCinToBinary<std::runtime_error>("Could not set standard input to binary mode (it defaults to text mode)");

    cin >> std::noskipws;

    Print_Header();

    std::copy( istream_iterator<char>(cin), istream_iterator<char>(), back_inserter(g_intact) );

    clogg << "Bytes: " << g_intact.size() << endl;
    clogg << "Lines: " << StringAlgorithms::Lines() << endl;

    std::replace(g_intact.begin(), g_intact.end(), '\0', ' ');  // null chars will screw up functions that parse null as end of string

    StringAlgorithms::replace_all(g_intact, "\r\n", " \n");

    StringAlgorithms::replace_all(g_intact, "\r", "\n");

    Replace_All_Preprocessor_Directives_With_Spaces();
    Replace_All_String_Literals_With_Spaces();

    //std::copy( g_intact.begin(), g_intact.end(), std::ostream_iterator<char>(clogg) );
    
    g_curly_manager.Process();

    clogg << "====================================== ALL PROCESSING DONE ===========================================" << endl;

    only_print_numbers = false;
    g_curly_manager.Print();

    //fifo_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    {
    Grade g(clogg, eGradeAllScopes);

    for ( auto const e : GetOpenSpacesBetweenInnerCurlyBrackets(g_curly_manager.Root()) )
    {
        Find_All_Usings_In_Open_Space(e.first, e.second, "::");
    }

    clogg << "====================================== All scope names ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        clogg << e.first << " [" << std::get<1u>(e.second) << "] ";

        for ( CurlyBracketManager::CurlyPair const *const  &my_curly_pair_pointer : std::get<0u>(e.second) )  // For classes, just one iteration. For namespaces, many iterations.
        {
            list< pair<size_t,size_t> > const my_list = GetOpenSpacesBetweenInnerCurlyBrackets(*my_curly_pair_pointer);

            for ( auto const my_pair : my_list )
            {
                auto &LineOf = StringAlgorithms::LineOf;

                {
                Grade g(clogg, eGradeOpenSpaces);
                clogg << " Open[" << LineOf(my_pair.first)+1u << "-" << LineOf(my_pair.second)+1u << "]";
                }

                Find_All_Usings_In_Open_Space(my_pair.first, my_pair.second, e.first + "::");
            }

            clogg << endl;
        }
    }
    }

    {
    Grade g(clogg, eGradeAllPseudonyms);
    clogg << "====================================== Now the pseudonyms (using X = Y;) ==============================================" << endl;
    for ( auto const &e : g_psuedonyms )
    {
        clogg << e.first << " == " << e.second << endl;
    }
    }

    {
    Grade g(clogg, eGradeAllNamespaces);
    clogg << "====================================== Now the namespaces ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        if ( "namespace" != std::get<1u>(e.second) ) continue;

        clogg << e.first << endl;
    }
    }

    {
    Grade g(clogg, eGradeAllDerivedClasses);
    clogg << "====================================== Now the classes that have base classes ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        if ( "struct" != std::get<1u>(e.second) && "class" != std::get<1u>(e.second) ) continue;

        list<string> const bases{ Get_All_Bases(e.first) };

        if ( bases.empty() ) continue;

        auto &LineOf = StringAlgorithms::LineOf;

        clogg << e.first << " - Line#" << LineOf(std::get<0u>(e.second).front()->First())+1u << " to Line#" << LineOf(std::get<0u>(e.second).front()->Last())+1u
              << " | Bases = ";

        for ( auto const &b : bases )
        {
            clogg << b << ", ";
        }

        clogg << endl;
    }
    }

    {
    Grade g(clogg, eGradeClassesWithContMarks);
    clogg << "====================================== Classes containing methods marked 'continue' ==============================================" << endl;
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
                    clogg << e.first << endl;

                    Print_Helper_Classes_For_Class(e.first);
                }
            }
        }
    }
    }

    // In reverse order
    Print_Final_Output();
}
