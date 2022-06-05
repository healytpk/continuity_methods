PROGRAM = precompiler

ifeq ($(OS),Windows_NT)
	#$(info Operating System detected as Microsoft)
	DYNAMO_OS = 1
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		#$(info Operating System detected as Apple)
		DYNAMO_OS = 2
	else ifeq ($(UNAME_S),Linux)
		#$(info Operating System detected as Linux)
		DYNAMO_OS = 3
	else
		$(error Could not detect operating system as one of Microsoft, Apple, Linux)
	endif
endif

.PHONY: all
all: $(PROGRAM)

$(PROGRAM):
	g++ -o precompiler -std=c++20 precompiler.cpp

.PHONY: clean
clean:
	$(RM) $(PROGRAM)
	#$(RM) -r dir_cppcheck_work dir_cppcheck_html build_log.txt

.PHONY: cppcheck
cppcheck:
	rm -rf dir_cppcheck_work dir_cppcheck_html
	mkdir -p dir_cppcheck_work dir_cppcheck_html
	(cd ./3rdparty/cppcheck/ && exec ./cppcheck --cppcheck-build-dir=../../dir_cppcheck_work/ --inline-suppr --force --library=std --library=boost --library=wxwidgets --xml -v --std=c++20 --enable=all '--suppress=*:/usr/include/*' $(PREPROCESSOR_DEFINES) ../../*.c ../../*.cpp 2> ../../err.xml)
	#cppcheck --cppcheck-build-dir=./dir_cppcheck_work/ --inline-suppr --force --library=std --library=boost --library=wxwidgets --xml -v --std=c++20 --enable=all '--suppress=*:/usr/include/*' $(PREPROCESSOR_DEFINES) ./*.c ./*.cpp 2> err.xml
	./3rdparty/cppcheck/cppcheck-htmlreport --file=err.xml --report-dir=./dir_cppcheck_html/ --source-dir=./

PHONY: prereq_cppcheck
prereq_cppcheck:
	apt update
	apt install -y python-pygments # required by cppcheck-htmlreport
	#wget launchpad.net/ubuntu/+archive/primary/+files/cppcheck_2.6-2_amd64.deb
	#dpkg -i cppcheck_2.6-2_amd64.deb

PHONY: prereq
prereq:
	apt update
	apt install -y gcc-11 g++-11
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11
	uname -a
	g++ --version | grep g++
