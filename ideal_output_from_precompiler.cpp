#include <iostream>
using std::cout;
using std::endl;

class Laser {
public:
    virtual bool Trigger(void)
    {
        cout << "Laser::Trigger" << endl;
        return true;
    }

    virtual void Elevate(float)
    {
        cout << "Laser::Elevate" << endl;
    }
};

class Laser_Nitrogen : virtual public Laser {
public:
    bool Trigger(void) override
    {
        cout << "Laser_Nitrogen::Trigger" << endl;
        return false;
    }
    
    void Elevate(float) override
    {
        cout << "Laser_Nitrogen::Elevate" << endl;
    }
};

class Laser_PicoSecond : virtual public Laser {
public:
    bool Trigger(void) override
    {
        cout << "Laser_PicoSecond::Trigger" << endl;
        return true;
    }
    
    void Elevate(float) override
    {
        cout << "Laser_PicoSecond::Elevate" << endl;
    }
};

class Laser_NitrogenPicoSecond : public Laser_Nitrogen, public
Laser_PicoSecond {
public:
    bool Trigger(void) override;
    void Elevate(float) override;
};

#include<array>

class IMethodInvoker {
protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    virtual bool Trigger(void*) = 0;
    virtual void Elevate(void*, float) = 0;

    friend class Invoker;
};

class Invoker final {
protected:

    IMethodInvoker &_mi;
    void *const _this;

public:

    Invoker(IMethodInvoker &arg_mi, void *const arg_this) : _mi(arg_mi),
_this(arg_this) {}

    // The extra 'this' parameter is no longer needed
    bool Trigger(void) // not virtual
    {
        return _mi.Trigger(_this);
    }
    
    // The extra 'this' parameter is no longer needed
    void Elevate(float const arg) // not virtual
    {
        return _mi.Elevate(_this,arg);
    }
};

template<class Base, class Derived>
class MethodInvoker final : public IMethodInvoker {
protected:

    union {
        void (*f)(void);
        bool (Base::*Trigger)(void);
        void (Base::*Elevate)(float);
    } const m;

public:

    MethodInvoker(void (*const arg_method)(void)) : m({arg_method}) {}

protected:

    // All methods have one extra
    // parameter for 'this' as 'void*'
    bool Trigger(void *const arg_this) override
    {
        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));  

        return (p->*m.Trigger)();
    }
 
    // All methods have one extra
    // parameter for 'this' as 'void*'
    void Elevate(void *const arg_this, float const arg0) override
    {
        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this));  

        return (p->*m.Elevate)(arg0);
    }
};

bool Laser_NitrogenPicoSecond::Trigger(void)
{    
    static MethodInvoker<Laser,Laser_NitrogenPicoSecond> mi_Laser( reinterpret_cast<void(*)(void)>(&Laser::Trigger) );
    
    static MethodInvoker<Laser_Nitrogen,Laser_NitrogenPicoSecond> mi_Laser_Nitrogen( reinterpret_cast<void(*)(void)>(&Laser_Nitrogen::Trigger) );
    
    static MethodInvoker<Laser_PicoSecond,Laser_NitrogenPicoSecond> mi_Laser_PicoSecond( reinterpret_cast<void(*)(void)>(&Laser_PicoSecond::Trigger) );
    
    std::array<Invoker,3u> methods = {
        Invoker(mi_Laser, this),
        Invoker(mi_Laser_Nitrogen, this),
        Invoker(mi_Laser_PicoSecond, this),
    };

    for ( auto &inv : methods )
    {
        inv.Trigger();

        //if ( inv->Trigger() ) return true;
    }
    
    cout << "Laser_NitrogenPicoSecond::Trigger" << endl;
    
    return false;
}

void Laser_NitrogenPicoSecond::Elevate(float const arg)
{    
    static MethodInvoker<Laser,Laser_NitrogenPicoSecond> mi_Laser( reinterpret_cast<void(*)(void)>(&Laser::Elevate) );
    
    static MethodInvoker<Laser_Nitrogen,Laser_NitrogenPicoSecond> mi_Laser_Nitrogen( reinterpret_cast<void(*)(void)>(&Laser_Nitrogen::Elevate) );
    
    static MethodInvoker<Laser_PicoSecond,Laser_NitrogenPicoSecond> mi_Laser_PicoSecond( reinterpret_cast<void(*)(void)>(&Laser_PicoSecond::Elevate) );
    
    std::array<Invoker,3u> methods = {
        Invoker(mi_Laser, this),
        Invoker(mi_Laser_Nitrogen, this),
        Invoker(mi_Laser_PicoSecond, this),
    };

    for ( auto &inv : methods )
    {
        inv.Elevate(arg);

        //if ( inv->Trigger() ) return true;
    }
    
    cout << "Laser_NitrogenPicoSecond::Elevate" << endl;
}

int main(void)
{
    cout << "=== First Line ===" << endl;
    Laser_NitrogenPicoSecond obj;
    obj.Trigger();
    obj.Elevate(2.2046f);
    cout << "=== Last Line ===" << endl;
}
