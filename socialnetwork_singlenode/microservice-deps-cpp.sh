#!/bin/bash
LIB_MONGOC_VERSION="1.14.0"
LIB_THRIFT_VERSION="0.12.0"
LIB_JSON_VERSION="3.6.1"
LIB_JAEGER_VERSION="0.4.2"
LIB_YAML_VERSION="0.6.2"
LIB_OPENTRACING_VERSION="1.5.1"
LIB_CPP_JWT_VERSION="1.1.1"
LIB_CPP_REDIS_VERSION="4.3.1"
LIB_AMQP_CPP_VERSION="4.1.4"
LIB_SIMPLEAMQPCLIENT_VERSION="2.4.0"

DIR="/proj/zyuxuanssf-PG0/tmp"

NUM_CPUS=1
BUILD_DEPS="ca-certificates g++ cmake wget git libmemcached-dev automake bison flex libboost-all-dev libevent-dev libssl-dev libtool make pkg-config librabbitmq-dev libhiredis-dev"

sudo apt-get update \
  && sudo apt-get install -y ${BUILD_DEPS} --no-install-recommends

rm -rf $DIR
mkdir $DIR

# Install mongo-c-driver
cd $DIR \
  && wget https://github.com/mongodb/mongo-c-driver/releases/download/${LIB_MONGOC_VERSION}/mongo-c-driver-${LIB_MONGOC_VERSION}.tar.gz \
  && tar -zxf mongo-c-driver-${LIB_MONGOC_VERSION}.tar.gz \
  && cd mongo-c-driver-${LIB_MONGOC_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=0 -DENABLE_EXAMPLES=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install lib-thrift
cd $DIR \
  && wget -O thrift-${LIB_THRIFT_VERSION}.tar.gz https://github.com/apache/thrift/archive/v${LIB_THRIFT_VERSION}.tar.gz \
  && tar -zxf thrift-${LIB_THRIFT_VERSION}.tar.gz \
  && cd thrift-${LIB_THRIFT_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install /nlohmann/json
cd $DIR \
  && wget -O json-${LIB_JSON_VERSION}.tar.gz https://github.com/nlohmann/json/archive/v${LIB_JSON_VERSION}.tar.gz \
  && tar -zxf json-${LIB_JSON_VERSION}.tar.gz \
  && cd json-${LIB_JSON_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install yaml-cpp
cd $DIR \
  && wget -O yaml-cpp-${LIB_YAML_VERSION}.tar.gz https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-${LIB_YAML_VERSION}.tar.gz \
  && tar -zxf yaml-cpp-${LIB_YAML_VERSION}.tar.gz \
  && cd yaml-cpp-yaml-cpp-${LIB_YAML_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fPIC" -DYAML_CPP_BUILD_TESTS=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install opentracing-cpp
cd $DIR \
  && wget -O opentracing-cpp-${LIB_OPENTRACING_VERSION}.tar.gz https://github.com/opentracing/opentracing-cpp/archive/v${LIB_OPENTRACING_VERSION}.tar.gz \
  && tar -zxf opentracing-cpp-${LIB_OPENTRACING_VERSION}.tar.gz \
  && cd opentracing-cpp-${LIB_OPENTRACING_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fPIC" -DBUILD_TESTING=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install jaeger-client-cpp
cd $DIR \
  && wget -O jaeger-client-cpp-${LIB_JAEGER_VERSION}.tar.gz https://github.com/jaegertracing/jaeger-client-cpp/archive/v${LIB_JAEGER_VERSION}.tar.gz \
  && tar -zxf jaeger-client-cpp-${LIB_JAEGER_VERSION}.tar.gz \
  && cd jaeger-client-cpp-${LIB_JAEGER_VERSION} \
  && mkdir -p cmake-build \
  && cd cmake-build \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fPIC" -DHUNTER_ENABLED=0 -DBUILD_TESTING=0 -DJAEGERTRACING_WITH_YAML_CPP=1 -DJAEGERTRACING_BUILD_EXAMPLES=0 .. \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install jwt
cd $DIR \
  && wget -O cpp-jwt-${LIB_CPP_JWT_VERSION}.tar.gz https://github.com/arun11299/cpp-jwt/archive/v${LIB_CPP_JWT_VERSION}.tar.gz \
  && tar -zxf cpp-jwt-${LIB_CPP_JWT_VERSION}.tar.gz \
  && cd cpp-jwt-${LIB_CPP_JWT_VERSION} \
  && sudo cp -R include/jwt /usr/local/include \
  && sudo rm -rf /usr/local/include/jwt/json \
  && sed 's/\#include \"jwt\/json\/json.hpp\"/\#include \<nlohmann\/json\.hpp\>/g'  /usr/local/include/jwt/jwt.hpp | sudo tee /usr/local/include/jwt/jwt.hpp
  #&& sed -i 's/\#include \"jwt\/json\/json.hpp\"/\#include \<nlohmann\/json\.hpp\>/g' /usr/local/include/jwt/jwt.hpp

# Install cpp_redis
cd $DIR \
  && git clone https://github.com/cpp-redis/cpp_redis.git \
  && cd cpp_redis && git checkout ${LIB_CPP_REDIS_VERSION} \
  && git submodule init && git submodule update \
  && mkdir cmake-build && cd cmake-build \
  && cmake .. -DCMAKE_BUILD_TYPE=Release \
  && make -j${NUM_CPUS} \
  && sudo make install

# Install AMQP-CPP
cd $DIR \
  && git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git \
  && cd AMQP-CPP && git checkout v${LIB_AMQP_CPP_VERSION} \
  && mkdir cmake-build && cd cmake-build \
  && cmake .. -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=on -DAMQP-CPP_LINUX_TCP=on \
  && make -j${NUM_CPUS} && sudo make install

# Install SimpleAmqpClient
cd $DIR \
  && git clone https://github.com/alanxz/SimpleAmqpClient.git \
  && cd SimpleAmqpClient \
  && git checkout v${LIB_SIMPLEAMQPCLIENT_VERSION} \
  && mkdir cmake-build && cd cmake-build \
  && cmake .. -DCMAKE_BUILD_TYPE=Release \
  && make -j${NUM_CPUS} \
  && sudo make install 

cd $DIR \
  && git clone https://github.com/redis/hiredis.git \
  && cd hiredis \
  && make \
  && sudo make install 
  && sudo ldconfig
#  && cd $ \
#  && rm -rf \
#    mongo-c-driver-${LIB_MONGOC_VERSION}.tar.gz \
#    mongo-c-driver-${LIB_MONGOC_VERSION} \
#    thrift-${LIB_THRIFT_VERSION}.tar.gz \
#    thrift-${LIB_THRIFT_VERSION} \
#    json-${LIB_JSON_VERSION}.tar.gz \
#    json-${LIB_JSON_VERSION} \
#    jaeger-client-cpp-${LIB_JAEGER_VERSION}.tar.gz \
#    jaeger-client-cpp-${LIB_JAEGER_VERSION} \
#    yaml-cpp-${LIB_YAML_VERSION}.tar.gz \
#    yaml-cpp-yaml-cpp-${LIB_YAML_VERSION} \
#    opentracing-cpp-${LIB_OPENTRACING_VERSION}.tar.gz \
#    opentracing-cpp-${LIB_OPENTRACING_VERSION} \
#    cpp-jwt-${LIB_CPP_JWT_VERSION}.tar.gz \
#    cpp-jwt-${LIB_CPP_JWT_VERSION} \
#    cpp_redis \
#    AMQP-CPP \
#    SimpleAmqpClient
