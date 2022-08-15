#include "myrpcwebserver.h"

int main(int argc, char *argv[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "zzh";
    string passwd = "131014";
    string databasename = "zzhdb";

    int zoo_port = 2181;
    string zoo_server_ip = "127.0.0.1";
    int server_port = 10002;



    WS *server = WS::getws();
    
    server->init(server_port,zoo_port,zoo_server_ip, user, passwd, databasename, 8, 8);
    

    //运行
    server->run();

    return 0;
}