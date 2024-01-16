#!/bin/bash
CUR_DIR=$(pwd)
ROOT_DIR="/DeathStarBench"
#ROOT_DIR=$CUR_DIR/../../..
echo $ROOT_DIR
THRIFT_GEN_CPP_DIR="$ROOT_DIR/socialNetwork/gen-cpp"
CPPFLAGS="-I/usr/local/include/jaegertracing -I$ROOT_DIR/thrift/lib/cpp/src -I$ROOT_DIR/socialNetwork/build/thrift -DBOOST_LOG_DYN_LINK"
LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lboost_log -lboost_log_setup -lboost_system -ljaegertracing -L/DeathStarBench/socialNetwork/build/lib" 
CC=/llvm-project/build/bin/clang++
LLVM_LINK=/llvm-project/build/bin/llvm-link
LLC=/llvm-project/build/bin/llc 

$CC -fPIC -emit-llvm -g -S $CPPFLAGS UniqueIdService.cpp -c -o UniqueIdService.ll
$CC -fPIC -emit-llvm -g -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/ComposePostService.cpp -c -o gen-ComposePostService.ll
$CC -fPIC -emit-llvm -g -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/social_network_types.cpp -c -o gen-social_network_types.ll
$CC -fPIC -emit-llvm -g -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/UniqueIdService.cpp -c -o gen-UniqueIdService.ll
$CC -fPIC -emit-llvm -g -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/PostStorageService.cpp -c -o gen-PostStorageService.ll
$CC -fPIC -emit-llvm -g -S $CPPFLAGS $THRIFT_GEN_CPP_DIR/UserTimelineService.cpp -c -o gen-UserTimelineService.ll
$LLVM_LINK UniqueIdService.ll gen-ComposePostService.ll gen-social_network_types.ll gen-UniqueIdService.ll gen-PostStorageService.ll gen-UserTimelineService.ll -o merged.ll -S
$LLC -filetype=obj -relocation-model=pic merged.ll -o merged.o
$CC -shared -fPIC -O2 $CPPFLAGS merged.o -o libUniqueIdService.so $LINKER_FLAGS


#$CC -shared -fPIC -O2 $CPPFLAGS  UniqueIdService.cpp $THRIFT_GEN_CPP_DIR/ComposePostService.cpp $THRIFT_GEN_CPP_DIR/social_network_types.cpp $THRIFT_GEN_CPP_DIR/UniqueIdService.cpp $THRIFT_GEN_CPP_DIR/PostStorageService.cpp $THRIFT_GEN_CPP_DIR/UserTimelineService.cpp  -o libUniqueIdService.so $LINKER_FLAGS

