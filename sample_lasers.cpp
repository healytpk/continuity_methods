#include <iostream>
using std::cout;
using std::endl;

class Laser {
public:
    void Trigger(void)
    {
        cout << "Set pin high to switch transistor to allow laser current" << endl;
    }
};

class Laser_Nitrogen : virtual public Laser {
public:
    void Trigger(void) continue
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
        /* nothing to do here */
    }
};

int main(void)
{
	Laser_NitrogenPicoSecond obj;

	obj.Trigger();
}
