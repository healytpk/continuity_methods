namespace Continuity_Methods {

    template<class T, T v>
    struct integral_constant {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant; // using injected-class-name
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; } // since c++14
    };

    using  true_type = integral_constant<bool,  true>;
    using false_type = integral_constant<bool, false>;

    template <class T, template <class...> class Test>
    struct exists {
        template<class U>
        static true_type check(Test<U>*);

        template<class U>
        static false_type check(...);

        static constexpr bool value = decltype(check<T>(0))::value;
    };

}

// ==========================================================================
// Helper classes for continuity methods within class ::My_Program::FurtherDerived
// ==========================================================================

namespace Continuity_Methods { namespace Helpers { namespace _scope_My_Program_scope_FurtherDerived {

namespace Testers {
    template<class U, class = decltype(&U::Set_Int____WITHOUT_CONTINUITY)>
    struct Set_Int____WITHOUT_CONTINUITY {};

    template<class U, class = decltype(&U::Set_Int)>
    struct Set_Int {};

}

class IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    virtual void Set_Int(void *const arg_this,int arg) = 0;

    friend class Invoker;
};

template<class Base, class Derived>
class MethodInvoker final : public IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    void Set_Int(void *const arg_this,int arg) override
    {
        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));

        if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_FurtherDerived::Testers::Set_Int____WITHOUT_CONTINUITY>::value )
        {
            return p->Base::Set_Int____WITHOUT_CONTINUITY(arg);
        }
        else if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_FurtherDerived::Testers::Set_Int>::value )
        {
            return p->Base::Set_Int(arg);
        }
        else
        {
            return;
        }
    }

};

class Invoker final {
protected:

    IMethodInvoker &_mi;
    void *const _this;

public:

    Invoker(IMethodInvoker &arg_mi, void *const arg_this)
      : _mi(arg_mi), _this(arg_this) {}

    // The extra 'this' parameter is no longer needed
    void Set_Int(int arg) // not virtual
    {
        return _mi.Set_Int(_this, arg);
    }

};

}}}  // close three namespaces

// ==========================================================================
// Helper classes for continuity methods within class ::My_Program::Derived2
// ==========================================================================

namespace Continuity_Methods { namespace Helpers { namespace _scope_My_Program_scope_Derived2 {

namespace Testers {
    template<class U, class = decltype(&U::Set_Int____WITHOUT_CONTINUITY)>
    struct Set_Int____WITHOUT_CONTINUITY {};

    template<class U, class = decltype(&U::Set_Int)>
    struct Set_Int {};

}

class IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    virtual void Set_Int(void *const arg_this,int const arg) = 0;

    friend class Invoker;
};

template<class Base, class Derived>
class MethodInvoker final : public IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    void Set_Int(void *const arg_this,int const arg) override
    {
        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));

        if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_Derived2::Testers::Set_Int____WITHOUT_CONTINUITY>::value )
        {
            return p->Base::Set_Int____WITHOUT_CONTINUITY(arg);
        }
        else if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_Derived2::Testers::Set_Int>::value )
        {
            return p->Base::Set_Int(arg);
        }
        else
        {
            return;
        }
    }

};

class Invoker final {
protected:

    IMethodInvoker &_mi;
    void *const _this;

public:

    Invoker(IMethodInvoker &arg_mi, void *const arg_this)
      : _mi(arg_mi), _this(arg_this) {}

    // The extra 'this' parameter is no longer needed
    void Set_Int(int const arg) // not virtual
    {
        return _mi.Set_Int(_this, arg);
    }

};

}}}  // close three namespaces

// ==========================================================================
// Helper classes for continuity methods within class ::My_Program::Derived1B
// ==========================================================================

namespace Continuity_Methods { namespace Helpers { namespace _scope_My_Program_scope_Derived1B {

namespace Testers {
    template<class U, class = decltype(&U::Set_Int____WITHOUT_CONTINUITY)>
    struct Set_Int____WITHOUT_CONTINUITY {};

    template<class U, class = decltype(&U::Set_Int)>
    struct Set_Int {};

}

class IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    virtual void Set_Int(void *const arg_this,int const arg) = 0;

    friend class Invoker;
};

template<class Base, class Derived>
class MethodInvoker final : public IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    void Set_Int(void *const arg_this,int const arg) override
    {
        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));

        if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_Derived1B::Testers::Set_Int____WITHOUT_CONTINUITY>::value )
        {
            return p->Base::Set_Int____WITHOUT_CONTINUITY(arg);
        }
        else if constexpr ( ::Continuity_Methods::exists<Base,::Continuity_Methods::Helpers::_scope_My_Program_scope_Derived1B::Testers::Set_Int>::value )
        {
            return p->Base::Set_Int(arg);
        }
        else
        {
            return;
        }
    }

};

class Invoker final {
protected:

    IMethodInvoker &_mi;
    void *const _this;

public:

    Invoker(IMethodInvoker &arg_mi, void *const arg_this)
      : _mi(arg_mi), _this(arg_this) {}

    // The extra 'this' parameter is no longer needed
    void Set_Int(int const arg) // not virtual
    {
        return _mi.Set_Int(_this, arg);
    }

};

}}}  // close three namespaces

#include <iostream>
using std::cout;
using std::endl;

namespace Caga { }

namespace My_Program {

int b;

struct Base1A {
    Base1A(void) { cout << "Base1A::Constructor" << endl; }

    virtual void Set_Int(int const arg)
    {
        cout << "Base1A value = " << arg << endl;
    }
};

struct Base1B {
    Base1B(void) { cout << "Base1B::Constructor" << endl; }

    void Set_Int(int const arg)
    {
        cout << "Base1B value = " << arg << endl;
    }
};

struct Derived1A : virtual Base1A {
    Derived1A(void) { cout << "Derived1A::Constructor" << endl; }

    void Set_Int(int const arg)
    {
        cout << "Derived1A value = " << arg << endl;
    }
};

struct Derived1B : virtual Base1A, Base1B {
    Derived1B(void) { cout << "Derived1B::Constructor" << endl; }

    void Set_Int____WITHOUT_CONTINUITY(int const arg)
    {
        cout << "Derived1B value = " << arg << endl;
    }

    void Set_Int(int const arg)
    {
        using namespace Continuity_Methods::Helpers::_scope_My_Program_scope_Derived1B;

        static MethodInvoker<::My_Program::Base1A, ::My_Program::Derived1B> mi__scope_My_Program_scope_Base1A;
        static MethodInvoker<::My_Program::Base1B, ::My_Program::Derived1B> mi__scope_My_Program_scope_Base1B;

        Invoker methods[2u] = {
            Invoker(mi__scope_My_Program_scope_Base1A, this),
            Invoker(mi__scope_My_Program_scope_Base1B, this),
        };

        for ( auto &e : methods )
        {
            e.Set_Int(arg);
        }

        this->Set_Int____WITHOUT_CONTINUITY(arg);
    }
};

struct Base2 {
    Base2(void) { cout << "Base2::Constructor" << endl; }

    // Doesn't have a method called "Set_Int"
};

struct Derived2 : virtual Derived1A,virtual Derived1B,virtual Base2 {
    Derived2(void) { cout << "Derived2::Constructor" << endl; }

    void Set_Int____WITHOUT_CONTINUITY(int const arg)
    {
        cout << "Derived2 value = " << arg << endl;
    }

    void Set_Int(int const arg) override
    {
        using namespace Continuity_Methods::Helpers::_scope_My_Program_scope_Derived2;

        static MethodInvoker<::My_Program::Base1A, ::My_Program::Derived2> mi__scope_My_Program_scope_Base1A;
        static MethodInvoker<::My_Program::Derived1A, ::My_Program::Derived2> mi__scope_My_Program_scope_Derived1A;
        static MethodInvoker<::My_Program::Base1B, ::My_Program::Derived2> mi__scope_My_Program_scope_Base1B;
        static MethodInvoker<::My_Program::Derived1B, ::My_Program::Derived2> mi__scope_My_Program_scope_Derived1B;
        static MethodInvoker<::My_Program::Base2, ::My_Program::Derived2> mi__scope_My_Program_scope_Base2;

        Invoker methods[5u] = {
            Invoker(mi__scope_My_Program_scope_Base1A, this),
            Invoker(mi__scope_My_Program_scope_Derived1A, this),
            Invoker(mi__scope_My_Program_scope_Base1B, this),
            Invoker(mi__scope_My_Program_scope_Derived1B, this),
            Invoker(mi__scope_My_Program_scope_Base2, this),
        };

        for ( auto &e : methods )
        {
            e.Set_Int(arg);
        }

        this->Set_Int____WITHOUT_CONTINUITY(arg);
    }
};

struct FurtherDerived : virtual Base2, Derived2  {
    FurtherDerived(void) { cout << "FurtherDerived::Constructor" << endl; }

    void Set_Int____WITHOUT_CONTINUITY(int arg)
    {
        cout << "FurtherDerived starts, value = " << arg << endl;

        for ( unsigned i = 0; i < (arg / 4); ++i ) cout << "monkey = " << ++arg << endl;

        cout << "FurtherDerived ends" << endl;
    }

    void Set_Int(int arg)
    {
        using namespace Continuity_Methods::Helpers::_scope_My_Program_scope_FurtherDerived;

        static MethodInvoker<::My_Program::Base2, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Base2;
        static MethodInvoker<::My_Program::Base1A, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Base1A;
        static MethodInvoker<::My_Program::Derived1A, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Derived1A;
        static MethodInvoker<::My_Program::Base1B, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Base1B;
        static MethodInvoker<::My_Program::Derived1B, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Derived1B;
        static MethodInvoker<::My_Program::Derived2, ::My_Program::FurtherDerived> mi__scope_My_Program_scope_Derived2;

        Invoker methods[6u] = {
            Invoker(mi__scope_My_Program_scope_Base2, this),
            Invoker(mi__scope_My_Program_scope_Base1A, this),
            Invoker(mi__scope_My_Program_scope_Derived1A, this),
            Invoker(mi__scope_My_Program_scope_Base1B, this),
            Invoker(mi__scope_My_Program_scope_Derived1B, this),
            Invoker(mi__scope_My_Program_scope_Derived2, this),
        };

        for ( auto &e : methods )
        {
            e.Set_Int(arg);
        }

        this->Set_Int____WITHOUT_CONTINUITY(arg);
    }
};

namespace Caga { }

namespace MyNamespace {

namespace Caga { }
using Frog = ::My_Program::FurtherDerived;
typedef ::My_Program::FurtherDerived Toad;

class Organism {};
class Mammal : Organism {};
class Canine : Mammal {};
class Wolf : Canine {};

class NonOrganism {

};

}
}

int main(void)
{
    cout << "=======================================================================================" << endl
         << "    START OF constructors for FurtherDerived" << endl
         << "=======================================================================================" << endl;

    My_Program::FurtherDerived a;

    cout << "=======================================================================================" << endl
         << "    START OF continuity methods for FurtherDerived" << endl
         << "=======================================================================================" << endl;

    a.Set_Int(7);

    cout << "\n\n=======================================================================================" << endl
         << "    START OF constructors for Derived1B" << endl
         << "=======================================================================================" << endl;

    My_Program::Derived1B b;

    cout << "=======================================================================================" << endl
         << "    START OF continuity methods for Derived1B" << endl
         << "=======================================================================================" << endl;

    b.Set_Int(7);
}
