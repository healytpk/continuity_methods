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
        std::cout << "Applying grade: " << (unsigned)arg << std::endl;
        previous = os.SetGrade(arg);
    }

    ~Grade(void)
    {
        std::cout << "Restoring grade: " << (unsigned)previous << std::endl;
        os.SetGrade(previous);
    }

    Grade(Grade const & ) = delete;
    Grade(Grade       &&) = delete;
    Grade &operator=(Grade const & ) = delete;
    Grade &operator=(Grade       &&) = delete;
    Grade const *operator&(void) const = delete;
    Grade       *operator&(void)       = delete;
};

using std::endl;

void Func2(void)
{
    Grade g(clogg, eGradeClassesWithContMarks);

    clogg << "Pig" << endl;
}

void Func1(void)
{
    Grade g(clogg, eGradeBanners);

    clogg << "Monkey" << endl;

    Func2();

    clogg << "Ape" << endl;
}

int main(void)
{
    Grade g(clogg,eGradeWarnings);

    clogg << "Hello World!" << endl;

    Func1();

    clogg << "Goodbye World!" << endl;
}
