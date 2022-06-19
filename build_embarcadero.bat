del precompiler.exe trans_*.cpp final_*.cpp bin_*.exe

set CXX="C:\Program Files\Embarcadero\RAD_Studio_11\bin\bcc64"

%CXX% -o precompiler.exe -std=c++17 precompiler.cpp import64.a cw64mt.a


%CXX% -o trans_lasers.cpp -E -P sample_lasers.cpp
type trans_lasers.cpp | precompiler.exe > final_lasers.cpp
%CXX% -o bin_lasers.exe -std=c++17 final_lasers.cpp import64.a cw64mt.a
bin_lasers.exe

%CXX% -o trans_simple.cpp -E -P sample_simple.cpp
type trans_simple.cpp | precompiler.exe > final_simple.cpp
%CXX% -o bin_simple.exe -std=c++17 final_simple.cpp import64.a cw64mt.a
bin_simple.exe
