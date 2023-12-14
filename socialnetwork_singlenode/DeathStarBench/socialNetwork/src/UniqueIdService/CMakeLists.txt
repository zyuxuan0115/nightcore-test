add_library(
    UniqueIdService SHARED
    UniqueIdService.cpp
    ${THRIFT_GEN_CPP_DIR}/UniqueIdService.cpp
    ${THRIFT_GEN_CPP_DIR}/ComposePostService.cpp
    ${THRIFT_GEN_CPP_DIR}/social_network_types.cpp
    # more files need to be added since we are to merge functions
    ${THRIFT_GEN_CPP_DIR}/PostStorageService.cpp
    ${THRIFT_GEN_CPP_DIR}/UserTimelineService.cpp
)

target_include_directories(
    UniqueIdService PRIVATE
    ${THRIFT_INCLUDE_DIRS}
    /usr/local/include/jaegertracing
)

target_link_libraries(
    UniqueIdService
    nlohmann_json::nlohmann_json
    thrift_static
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    Boost::log
    Boost::log_setup
    jaegertracing
    # more libraries need to be added since we are to merge functions
    OpenSSL::SSL
    /usr/local/lib/libjaegertracing.so
    /usr/local/lib/libhiredis.a
    /usr/local/lib/libSimpleAmqpClient.so
)

install(TARGETS UniqueIdService DESTINATION ./)
