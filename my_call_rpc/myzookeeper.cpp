#include "myzookeeper.h"

void ZkClient::Start(){
    string connstr = m_zoo_server_ip+":"+std::to_string(m_zoo_port);
    m_zhandle = zookeeper_init(connstr.c_str(),init_watcher,
    30000,nullptr,nullptr,0);
    
    if(m_zhandle == nullptr){
        LOG_ERROR("zoo init error");
        exit(-1);
    }
    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);
    sem_wait(&sem);
    LOG_INFO("zoo connect success");

}

void ZkClient::Create(const char *path,const char *data, int datalen,
int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle,path,0,nullptr);
    if(ZNONODE == flag){
        flag = zoo_create(m_zhandle,path,data,
        datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag == ZOK){
            LOG_INFO("zoo create success");
        }else{
            LOG_ERROR("zoo create error");
            exit(-1);
        }
    }
}
string ZkClient::GetData(const char *path){
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle,path,0,buffer,
    &bufferlen,nullptr);
    if(flag == ZOK){
        LOG_INFO("zoo get success");
        return buffer;
    }else{
        LOG_ERROR("zoo get error");
        return "";
    }
}