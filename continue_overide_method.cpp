#include <iostream>
using std::cout;
using std::endl;

#define requires /* nothing */
#define continue /* nothing */

class Laser {
public:
    void Trigger(void) requires continue
    {
        cout << "Set pin high to switch transistor to allow laser current" << endl;
    }
};

class Laser_Nitrogen : virtual public Laser {
public:
    void Trigger(void) continue override
    {
        cout << "Set TTL pin high to activate nitrogen laser" << endl;
    }
};

class Laser_PicoSecond : virtual public Laser {
public:
    void Trigger(void) continue
    {
        cout << "Send trigger command to the PicoSecond laser over RS232" << endl;
    }
};

class Laser_NitrogenPicoSecond : public Laser_Nitrogen, public Laser_PicoSecond {
public:
    void Trigger(void) continue
    {
		/* nothing */
    }
};

#include <cstdlib>   // EXIT_FAILURE

int main(void)
{
    Laser_NitrogenPicoSecond object;
    
    if ( static_cast<Laser*>(static_cast<Laser_Nitrogen*>(&object)) != static_cast<Laser*>(static_cast<Laser_PicoSecond*>(&object)) )
    {
		cout << "Something's wrong here -- there should on be on Laser object" << endl;
		return EXIT_FAILURE;
	}

    object.Trigger();
}
