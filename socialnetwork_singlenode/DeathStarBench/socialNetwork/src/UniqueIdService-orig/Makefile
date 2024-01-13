ROOT_DIR=/proj/zyuxuanssf-PG0/nightcore-test/socialnetwork_singlenode/DeathStarBench/socialNetwork
THRIFT_GEN_CPP_DIR=$(ROOT_DIR)/gen-cpp
CPPFLAGS=-I/usr/local/include/jaegertracing -I$(ROOT_DIR)/thrift/lib/cpp/src -I$(ROOT_DIR)/thrift
LINKER_FLAGS=-L/usr/local/lib -lboost_log -lboost_log_setup -ljaegertracing 

all:
	g++ -shared -fPIC -O2 $(CPPFLAGS)  UniqueIdService.cpp $(THRIFT_GEN_CPP_DIR)/ComposePostService.cpp $(THRIFT_GEN_CPP_DIR)/social_network_types.cpp $(THRIFT_GEN_CPP_DIR)/UniqueIdService.cpp -o UniqueIdService.so $(LINKER_FLAGS)

#UniqueIdService.o:
#	g++ -c -fPIC -O2 $(CPPFLAGS)  UniqueIdService.cpp $(THRIFT_GEN_CPP_DIR)/ComposePostService.cpp $(THRIFT_GEN_CPP_DIR)/social_network_types.cpp $(THRIFT_GEN_CPP_DIR)/UniqueIdService.cpp 

clean:
	rm UniqueIdService.so
