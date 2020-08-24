# Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

# Usage examples (assuming this directory is ~/src/atomic_queue):
# time make -rC ~/src/optparse -j8 run_test
# time make -rC ~/src/optparse -j8 TOOLSET=clang BUILD=debug run_tes

SHELL := /bin/bash
BUILD := release

TOOLSET := gcc
build_dir := ${CURDIR}/build/${BUILD}/${TOOLSET}

cxx.gcc := g++
cc.gcc := gcc
ld.gcc := g++
ar.gcc := gcc-ar

cxx.clang := clang++
cc.clang := clang
ld.clang := clang++
ar.clang := ar

CXX := ${cxx.${TOOLSET}}
CC := ${cc.${TOOLSET}}
LD := ${ld.${TOOLSET}}
AR := ${ar.${TOOLSET}}

cxxflags.gcc.debug := -Og -fstack-protector-all -fno-omit-frame-pointer # -D_GLIBCXX_DEBUG
cxxflags.gcc.release := -O3 -mtune=native -ffast-math -falign-{functions,loops}=64 -DNDEBUG
cxxflags.gcc := -pthread -march=native -std=gnu++17 -W{all,extra,error,no-{maybe-uninitialized,unused-function,unused-local-typedefs}} -g -fmessage-length=0 ${cxxflags.gcc.${BUILD}}

cflags.gcc := -pthread -march=native -W{all,extra} -g -fmessage-length=0 ${cxxflags.gcc.${BUILD}}

cxxflags.clang.debug := -O0 -fstack-protector-all
cxxflags.clang.release := -O3 -mtune=native -ffast-math -falign-functions=64 -DNDEBUG
cxxflags.clang := -stdlib=libstdc++ -pthread -march=native -std=gnu++17 -W{all,extra,error,no-{unused-variable,unused-function,unused-local-typedefs}} -g -fmessage-length=0 ${cxxflags.clang.${BUILD}}
ldflags.clang := -stdlib=libstdc++ ${ldflags.clang.${BUILD}}

# Additional CPPFLAGS, CXXFLAGS, CFLAGS, LDLIBS, LDFLAGS can come from the command line, e.g. make CPPFLAGS='-I<my-include-dir>', or from environment variables.
# However, a clean build is required when changing the flags in the command line or in environment variables, this makefile doesn't detect such changes.
cxxflags := ${cxxflags.${TOOLSET}} ${CXXFLAGS}
cflags := ${cflags.${TOOLSET}} ${CFLAGS}
cppflags := ${CPPFLAGS} -Iinclude
ldflags := -fuse-ld=gold -pthread -g ${ldflags.${TOOLSET}} ${LDFLAGS}
ldlibs := -lrt ${LDLIBS}

ifdef BOOST_ROOT_1_72_0 # E.g./opt/hostedtoolcache/boost/1.72.0/x64
boost_unit_test_framework_inc := -I${BOOST_ROOT_1_72_0}
boost_unit_test_framework_lib := -{L,'Wl,-rpath='}${BOOST_ROOT_1_72_0}/lib -lboost_unit_test_framework-mt-x64
else
boost_unit_test_framework_lib := -lboost_unit_test_framework
endif

COMPILE.CXX = ${CXX} -o $@ -c ${cppflags} ${cxxflags} -MD -MP $(abspath $<)
COMPILE.S = ${CXX} -o- -S -masm=intel ${cppflags} ${cxxflags} $(abspath $<) | c++filt | egrep -v '^[[:space:]]*\.(loc|cfi|L[A-Z])' > $@
PREPROCESS.CXX = ${CXX} -o $@ -E ${cppflags} ${cxxflags} $(abspath $<)
COMPILE.C = ${CC} -o $@ -c ${cppflags} ${cflags} -MD -MP $(abspath $<)
LINK.EXE = ${LD} -o $@ $(ldflags) $(filter-out Makefile,$^) $(ldlibs)
LINK.SO = ${LD} -o $@ -shared $(ldflags) $(filter-out Makefile,$^) $(ldlibs)
LINK.A = ${AR} rscT $@ $(filter-out Makefile,$^)

exes := test

all : ${exes}

${exes} : % : ${build_dir}/%
	ln -sf ${<:${CURDIR}/%=%}

test_src := test.cc
${build_dir}/test : cppflags += ${boost_unit_test_framework_inc} -DBOOST_TEST_DYN_LINK=1
${build_dir}/test : ldlibs += ${boost_unit_test_framework_lib}
${build_dir}/test : ${test_src:%.cc=${build_dir}/%.o} ${build_dir}/libcoptpase.a Makefile | ${build_dir}
	$(strip ${LINK.EXE})
-include ${test_src:%.cc=${build_dir}/%.d}

example_src := example.cc
${build_dir}/example : ${example_src:%.cc=${build_dir}/%.o} ${build_dir}/libcoptpase.a Makefile | ${build_dir}
	$(strip ${LINK.EXE})
-include ${example_src:%.cc=${build_dir}/%.d}

libcoptpase_src := optparse.cc
${build_dir}/libcoptpase.a : ${libcoptpase_src:%.cc=${build_dir}/%.o} Makefile | ${build_dir}
	$(strip ${LINK.A})
-include ${libcoptpase_src:%.cc=${build_dir}/%.d}

${build_dir}/%.so : cxxflags += -fPIC
${build_dir}/%.so : Makefile | ${build_dir}
	$(strip ${LINK.SO})

${build_dir}/%.a : Makefile | ${build_dir}
	$(strip ${LINK.A})

run_% : ${build_dir}/%
	@echo "---- running $< ----"
	$<

${build_dir}/%.o : src/%.cc Makefile | ${build_dir}
	$(strip ${COMPILE.CXX})

${build_dir}/%.o : src/%.c Makefile | ${build_dir}
	$(strip ${COMPILE.C})

%.S : src/%.cc Makefile | ${build_dir}
	$(strip ${COMPILE.S})

%.I : src/%.cc
	$(strip ${PREPROCESS.CXX})

${build_dir} :
	mkdir -p $@

rtags : clean
	${MAKE} -nk | rc -c -; true

clean :
	rm -rf ${build_dir} ${exes}

env :
	env | sort

versions:
	${MAKE} --version | head -n1
	${CXX} --version | head -n1

.PHONY : env versions rtags run_benchmarks clean all run_%
