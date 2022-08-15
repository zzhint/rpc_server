#!/bin/bash

g++ main.cpp myepoll.cpp myrpc.cpp myrpcwebserver.cpp mysqlpool.cpp  \
myzookeeper.cpp rpcheader.pb.cc user.pb.cc log.cpp myservice.cpp -o main \
-lprotobuf -lpthread -lzookeeper_mt -pthread -lmysqlclient