#!/bin/bash
CUR_DIR=$(pwd)

#ROOT_DIR="/DeathStarBench"
#LLVM_BUILD_PATH=/llvm-project-10/build

ROOT_DIR=/proj/zyuxuanssf-PG0/nightcore-test/socialnetwork_softbound/DeathStarBench
LLVM_BUILD_PATH=/proj/zyuxuanssf-PG0/llvm-project-10/build

#ROOT_DIR=$CUR_DIR/../../..
THRIFT_GEN_CPP_DIR="$ROOT_DIR/socialNetwork/gen-cpp"
CPPFLAGS="-I/usr/local/include/jaegertracing -I$ROOT_DIR/thrift/lib/cpp/src -I$ROOT_DIR/socialNetwork/build/thrift -DBOOST_LOG_DYN_LINK"
LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lboost_log -lboost_log_setup -lboost_system -ljaegertracing -L$ROOT_DIR/socialNetwork/build/lib" 
CC=$LLVM_BUILD_PATH/bin/clang++
LLVM_LINK=$LLVM_BUILD_PATH/bin/llvm-link
LLC=$LLVM_BUILD_PATH/bin/llc 
OPT=$LLVM_BUILD_PATH/bin/opt
SOFTBOUND_INIT_LIB=$LLVM_BUILD_PATH/lib/InitializeSoftBoundCETS.so
SOFTBOUND_LIB=$LLVM_BUILD_PATH/lib/LLVMSoftBoundCETS.so

rm -rf *.so *.ll *.o tmp

# compile all source files into IR
$CC -fPIC -emit-llvm -S $CPPFLAGS UniqueIdService.cpp -c -o UniqueIdService.ll
$CC -fPIC -emit-llvm -S $CPPFLAGS $CUR_DIR/../ComposePostService/ComposePostService.cpp -c -o ComposePostService.ll
$CC -fPIC -emit-llvm -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/ComposePostService.cpp -c -o gen-ComposePostService.ll
$CC -fPIC -emit-llvm -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/social_network_types.cpp -c -o gen-social_network_types.ll
$CC -fPIC -emit-llvm -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/UniqueIdService.cpp -c -o gen-UniqueIdService.ll

$LLVM_LINK UniqueIdService.ll gen-ComposePostService.ll gen-social_network_types.ll gen-UniqueIdService.ll -o merged.ll -S

# get all functions in UniqueIdService
$OPT -load $SOFTBOUND_INIT_LIB -InitializeSoftBoundCETS merged.ll -S -o merged_init.ll
$OPT -load $SOFTBOUND_INIT_LIB -SoftBoundCETSPass merged_init.ll -S -o merged_softbound.ll


$LLC -filetype=obj -relocation-model=pic merged_softbound.ll -o merged.o
#$CC -shared -fPIC -O2 $CPPFLAGS merged.o -o libUniqueIdService.so $LINKER_FLAGS


#$CC -shared -fPIC -O2 $CPPFLAGS  UniqueIdService.cpp $THRIFT_GEN_CPP_DIR/ComposePostService.cpp $THRIFT_GEN_CPP_DIR/social_network_types.cpp $THRIFT_GEN_CPP_DIR/UniqueIdService.cpp $THRIFT_GEN_CPP_DIR/PostStorageService.cpp $THRIFT_GEN_CPP_DIR/UserTimelineService.cpp  -o libUniqueIdService.so $LINKER_FLAGS

