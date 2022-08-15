#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <string>
#include "mysem.h"
#include <mutex>

using std::string;


class Sql_pool
{

public:
	
    static Sql_pool* getsql(){
        static Sql_pool s;
        return &s;
    }

    void init(string url, string User, 
    string PassWord, string DataBaseName, 
    int Port, int MaxConn);
    MYSQL *get_one();
    void push_one(MYSQL *con);
private:
    Sql_pool(){};
	~Sql_pool();
	int m_MaxConn;  //最大连接数
	int m_CurConn=0;  //当前已使用的连接数
	int m_FreeConn=0; //当前空闲的连接数
	std::mutex lock;
	std::list<MYSQL *> sqlpool; //连接池
	sem m_sem;

public:
	string m_url;			 //主机地址
	string m_Port;		 //数据库端口号
	string m_User;		 //登陆数据库用户名
	string m_PassWord;	 //登陆数据库密码
	string m_DatabaseName; //使用数据库名
	int m_close_log;	//日志开关
};


#endif
