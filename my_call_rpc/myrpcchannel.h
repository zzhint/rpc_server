#ifndef MYRPCCHANNEL_H
#define MYRPCCHANNEL_H
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include "myzookeeper.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "log.h"
using std::string;
class Myrpcchannel : public google::protobuf::RpcChannel{
public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                google::protobuf::RpcController *controller,
                const google::protobuf::Message *request,
                google::protobuf::Message *response,
                google::protobuf::Closure *done);
};





#endif 