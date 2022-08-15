#include "mysqlpool.h"
#include <unordered_map>
extern std::unordered_map<string,string> users;


void Sql_pool::init(string url, string User, 
string PassWord, string DataBaseName, 
int Port, int MaxConn)
{//转而使用条件变量
    for(int i=0;i<MaxConn;i++){
        MYSQL *con = NULL;
        con = mysql_init(con);
        con = mysql_real_connect(con,
        url.c_str(), User.c_str(), 
        PassWord.c_str(), DataBaseName.c_str(), 
        Port, NULL, 0);
        sqlpool.push_back(con);
        ++m_FreeConn;
        
    }
    m_MaxConn = MaxConn;
    m_sem = sem(MaxConn);
    Sql_pool *sl = Sql_pool::getsql();
    MYSQL *mysql = sl->get_one();
    mysql_query(mysql,"SELECT username,passwd FROM user");
    MYSQL_RES *result = mysql_store_result(mysql);
    
    MYSQL_ROW row;
    string uname;
    string password;
    while (row = mysql_fetch_row(result))
    {
        uname = row[0];
        password = row[1];
        users[uname] = password;
    }
    sl->push_one(mysql);


}


Sql_pool::~Sql_pool()
{
lock.lock();
for(auto it = sqlpool.begin();it!=sqlpool.end();it++){
    MYSQL *con = *it;
    mysql_close(con);
}
sqlpool.clear();
lock.unlock();
}

MYSQL *Sql_pool::get_one()
{
    MYSQL *con;
    m_sem.wait();
    lock.lock();

    con = sqlpool.front();
    sqlpool.pop_front();

    lock.unlock();
    return con;
}

void Sql_pool::push_one(MYSQL *con){
    lock.lock();
    sqlpool.push_back(con);
    lock.unlock();
    m_sem.post();
}