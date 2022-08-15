#include "mywebserver.h"

int main(int argc, char *argv[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "zzh";
    string passwd = "131014";
    string databasename = "zzhdb";

    int port = 10001;
    WS *server = WS::getws();
    
    //初始化

    server->init(port, user, passwd, databasename, 8, 8);
    

    //运行
    server->eventLoop();

    return 0;
}