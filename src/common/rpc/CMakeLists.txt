cmake_minimum_required(VERSION 3.3)

set(SOURCE_FILES proto/message.pb.cc proto/message.pb.h)
add_library(rpcMessages ${SOURCE_FILES})
