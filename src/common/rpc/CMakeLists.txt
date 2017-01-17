cmake_minimum_required(VERSION 3.3)

#TODO: change this into an auto-generated version (call shell script? the standard method doesn't seem to work)
set(SOURCE_FILES proto/message.pb.cc proto/message.pb.h proto/oss_server_message.pb.h proto/oss_server_message.pb.cc)
add_library(rpcMessages ${SOURCE_FILES})
