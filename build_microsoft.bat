call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"

set compiler="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.32.31326\bin\Hostx64\x64\cl.exe"

%compiler% /Fe:precompiler.exe ./precompiler.cpp /std:c++20 /EHsc

%compiler% /EP sample_simple.cpp /std:c++20 | precompiler.exe > trans_simple.cpp
%compiler% /Fe:bin_simple.exe ./trans_simple.cpp /std:c++20 /EHsc
bin_simple.exe

%compiler% /EP sample_simple.cpp /std:c++20 | precompiler.exe > trans_simple.cpp
%compiler% /Fe:bin_simple.exe ./trans_simple.cpp /std:c++20 /EHsc
bin_simple.exe
