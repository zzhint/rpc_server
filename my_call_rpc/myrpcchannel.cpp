#include "myrpcchannel.h"

void Myrpcchannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                google::protobuf::RpcController *controller,
                const google::protobuf::Message *request,
                google::protobuf::Message *response,
                google::protobuf::Closure *done)
{
    LOG_INFO("serialize request data");
    const google::protobuf::ServiceDescriptor *sd = method->service();
    string service_name = sd->name();
    string method_name = method->name();

    uint32_t args_size = 0;
    string args_str;

    if(request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }else{
        LOG_ERROR("request can't be serialize");
        return;
    }

    mprpc::RpcHeader rpchd;
    rpchd.set_service_name(service_name);
    rpchd.set_method_name(method_name);
    rpchd.set_args_size(args_size);

    uint32_t header_size = 0;
    string rpc_header_str;
    if(rpchd.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }else{
        LOG_ERROR("rpchd can't be serialize");
        return;
    }
    string send_rpc_str((char *)&header_size,4);
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;


    LOG_INFO("connecting...");
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == clientfd){
        LOG_ERROR("socket error");
        return;
    }
    ZkClient zkCli;
    zkCli.Start();
    string method_path = "/" + service_name + "/" +
    method_name;
    
    string host_data = zkCli.GetData(method_path);
    if (host_data == ""){
        LOG_ERROR("zoo has not service");
        return;
    }
    int idx = host_data.find(":");
    string ip = host_data.substr(0,idx);
    uint16_t port = std::stoi(host_data.substr(idx+1,host_data.size()-idx));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(-1 == connect(clientfd, (struct sockaddr *)&server_addr,sizeof(server_addr))){
        LOG_ERROR("connect error");
        return;
    }
    if(-1 == send(clientfd, send_rpc_str.c_str(),send_rpc_str.size()
    ,0)){
        LOG_ERROR("send error");
        return;
    }

    char recv_buf[1024] = {0};//?
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0))){
        LOG_ERROR("recv error");
        return;
    }
    LOG_INFO("parse response");
    if(!response->ParseFromArray(recv_buf,recv_size)){
        LOG_ERROR("response parse error");
        return;
    }
    close(clientfd);
    
}