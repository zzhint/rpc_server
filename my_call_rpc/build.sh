#!/bin/bash

g++ main.cpp myepoll.cpp mywebserver.cpp \
myzookeeper.cpp rpcheader.pb.cc user.pb.cc log.cpp myhttp.cpp \
myrpcchannel.cpp  -o main \
-lprotobuf -lpthread -lzookeeper_mt -pthread -lmysqlclient