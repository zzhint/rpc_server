#ifndef MYZOOKEEPER_H
#define MYZOOKEEPER_H


#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <iostream>
#include "log.h"
using std::string;
// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient(string zoo_server_ip="127.0.0.1",int zoo_port=2181):m_zoo_server_ip(zoo_server_ip)
    ,m_zoo_port(zoo_port){};
    ~ZkClient(){
        if (m_zhandle != nullptr){
            zookeeper_close(m_zhandle);
        }
    };
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int datalen, int state=0);
    // 根据参数指定的znode节点路径，或者znode节点的值
    string GetData(const char *path);
    string GetData(string path){
        return GetData(path.c_str());
    }
    
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle = nullptr;
    string m_zoo_server_ip;
    int m_zoo_port;
    static void init_watcher(zhandle_t *zh,int type,int state,
    const char *path, void *watcherCtx){
        if(type == ZOO_SESSION_EVENT){
            if (state == ZOO_CONNECTED_STATE){
                sem_t *sem = (sem_t*) zoo_get_context(zh);
                sem_post(sem);
            }
        }
    }
};


#endif