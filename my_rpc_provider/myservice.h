#ifndef MYSERVICE_H
#define MYSERVICE_H
#include "user.pb.h"
#include "rpcheader.pb.h"
#include "log.h"
#include "mysqlpool.h"
class my_service : public fixbug::UserServiceRpc{
public:
    static std::mutex m_mutex;
    MYSQL *m_mysql;
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done);
    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done);
};



#endif