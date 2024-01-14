#!/bin/bash
CUR_DIR=$(pwd)
#ROOT_DIR="/proj/zyuxuanssf-PG0/nightcore-test/socialnetwork_singlenode/DeathStarBench/socialNetwork"
ROOT_DIR=$CUR_DIR/../..
echo $ROOT_DIR
THRIFT_GEN_CPP_DIR="$ROOT_DIR/gen-cpp"
CPPFLAGS="-I/usr/local/include/jaegertracing -I$ROOT_DIR/thrift/lib/cpp/src -I$ROOT_DIR/build/thrift"
LINKER_FLAGS="-L/usr/lib/x86_64-linux-gnu -lboost_log -lboost_log_setup -lboost_system -ljaegertracing -L/DeathStarBench/socialNetwork/build/lib" 

g++ -shared -fPIC -O2 $CPPFLAGS  UniqueIdService.cpp $THRIFT_GEN_CPP_DIR/ComposePostService.cpp $THRIFT_GEN_CPP_DIR/social_network_types.cpp $THRIFT_GEN_CPP_DIR/UniqueIdService.cpp -o libUniqueIdService.so $LINKER_FLAGS

