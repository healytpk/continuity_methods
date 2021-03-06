#!/bin/sh

rm -rf precompiler transunit_* final_* bin_*

COMMAND_BUILD_PRECOMPILER="g++ -o precompiler precompiler.cpp -std=c++20 -fdump-rtl-expand "

if [ "$1" == "--release" ]; then
    COMMAND_BUILD_PRECOMPILER+=" -DNDEBUG -O3 -s "
else
    export ASAN_OPTIONS="detect_invalid_pointer_pairs=2"
    COMMAND_BUILD_PRECOMPILER+=" -ggdb3 -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract,float-divide-by-zero,float-cast-overflow \
                                 -fsanitize-address-use-after-scope -fsanitize-coverage=trace-cmp \
                                 -fcf-protection=full -fstack-protector-all -fstack-clash-protection \
                                 -fvtv-debug -fvtv-counts -finstrument-functions "
fi

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

COMMAND_PRECOMPILER="| ./precompiler 2> /dev/null"
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
/opt/clang/bin/clang++ -o clang_precompiler_libstdc++ -std=c++20 -stdlib=libstdc++ ./precompiler.cpp
/opt/clang/bin/clang++ -std=c++20 -stdlib=libstdc++ -E -P sample_lasers.cpp | ./clang_precompiler_libstdc++ 2> /dev/null | /opt/clang/bin/clang++ -o bin_clang_lasers_libstdc++ -std=c++20 -stdlib=libstdc++ -x c++ -
echo "= = = = = Attempting to build with the clang compiler (with 'libc++'). . . "
/opt/clang/bin/clang++ -o clang_precompiler_libc++ -std=c++20 -stdlib=libc++ ./precompiler.cpp
/opt/clang/bin/clang++ -std=c++20 -stdlib=libc++ -E -P sample_lasers.cpp | ./clang_precompiler_libc++ 2> /dev/null | /opt/clang/bin/clang++ -o bin_clang_lasers_libc++ -std=c++20 -stdlib=libc++ -x c++ -

