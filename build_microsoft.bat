call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"

set compiler="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.32.31326\bin\Hostx64\x64\cl.exe"

%compiler% /Fe:precompiler.exe ./precompiler.cpp /std:c++20 /EHsc
