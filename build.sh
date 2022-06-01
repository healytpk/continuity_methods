#!/bin/sh

rm -rf prog transunit_* final_* bin_*

COMMAND_BUILD_PRECOMPILER="g++ -o prog -std=c++20 precompiler.cpp -ggdb3 -Og"
echo "= = = = = Building 'precompiler.cpp' . . . "
echo ${COMMAND_BUILD_PRECOMPILER}
${COMMAND_BUILD_PRECOMPILER}

if [ $? -eq 0 ]
then
    echo "= = = = = Build successful"
else
    echo "! ! ! ! ! Build failed"
    exit 1
fi

COMMAND_SOURCE_TO_TRANSLATION_UNIT="g++ -E -P -std=c++20"
find -name "sample_*.cpp" | xargs -r -n1 basename | while read filename;
do
	echo "= = = = = Producing translation unit from original source file. . . "
	echo "${COMMAND_SOURCE_TO_TRANSLATION_UNIT} ${filename} > transunit_${filename}"
	${COMMAND_SOURCE_TO_TRANSLATION_UNIT} ${filename} > transunit_${filename}

	if [ $? -eq 0 ]
	then
	    echo "= = = = = Translation unit production successful - FROM: ${filename} TO: transunit_${filename}"
	else
	    echo "! ! ! ! ! Translation unit production failed - FROM: ${filename} TO: transunit_${filename}"
	    exit 1
	fi
done

COMMAND_PRECOMPILER="| ./prog 2> /dev/null"
find -name "transunit_*.cpp" | xargs -r -n1 basename | while read filename;
do
    echo "= = = = = Producing C++20 source file from translation unit. . . "
    echo "cat ${filename} ${COMMAND_PRECOMPILER} > final_${filename}"
    sh -c "cat ${filename} ${COMMAND_PRECOMPILER} > final_${filename}"

    if [ $? -eq 0 ]
    then
        echo "= = = = = Precompilation successful - FROM: ${filename} TO: final_${filename}"
    else
        echo "! ! ! ! ! Precompilation failed - FROM: ${filename} TO: final_${filename}"
        exit 1
    fi
done

COMMAND_FINAL_BUILD="g++ --std=c++20 -o"
find -name "final_*.cpp" | xargs -r -n1 basename | while read filename;
do
    echo "= = = = = Producing exectuable program from final C++20 source file. . . "
    echo "${COMMAND_FINAL_BUILD} bin_${filename} ${filename}"
    sh -c "${COMMAND_FINAL_BUILD} bin_${filename} ${filename}"

    if [ $? -eq 0 ]
    then
        echo "= = = = = Final compilation successful - FROM: ${filename} TO: bin_${filename}"
    else
        echo "! ! ! ! ! Final compilation failed - FROM: ${filename} TO: bin_${filename}"
        exit 1
    fi
done

echo "= = = = = Attempting to build with the clang compiler (with 'libstdc++'). . . "
/opt/clang/bin/clang++ -o clang_prog_libstdc++ -std=c++20 -stdlib=libstdc++ ./precompiler.cpp
/opt/clang/bin/clang++ -std=c++20 -stdlib=libstdc++ -E -P sample_lasers.cpp | ./clang_prog_libstdc++ 2> /dev/null | /opt/clang/bin/clang++ -o bin_clang_lasers_libstdc++ -std=c++20 -stdlib=libstdc++ -x c++ -
echo "= = = = = Attempting to build with the clang compiler (with 'libc++'). . . "
/opt/clang/bin/clang++ -o clang_prog_libc++ -std=c++20 -stdlib=libc++ ./precompiler.cpp
/opt/clang/bin/clang++ -std=c++20 -stdlib=libc++ -E -P sample_lasers.cpp | ./clang_prog_libc++ 2> /dev/null | /opt/clang/bin/clang++ -o bin_clang_lasers_libc++ -std=c++20 -stdlib=libc++ -x c++ -

