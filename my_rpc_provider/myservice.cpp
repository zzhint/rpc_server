#include "myservice.h"
#include "mysqlpool.h"
extern std::unordered_map<string,string>  users;

void my_service::Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
{

    std::string name = request->name();
    std::string pwd = request->pwd();
    fixbug::ResultCode *code = response->mutable_result();
    if(users.find(name) == users.end()){
        code->set_errcode(1);
        code->set_errmsg("there isn't you");
        response->set_sucess(false);
    }else if(users[name] != pwd){
        code->set_errcode(2);
        code->set_errmsg("password error");
        response->set_sucess(false);
    }else{
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(true);
    }

}
void my_service::Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
{

    std::string name = request->name();
    std::string pwd = request->pwd();
    fixbug::ResultCode *code = response->mutable_result();
    string sql_insert = "INSERT INTO user(username, passwd) VALUES('";
    sql_insert+=name;
    sql_insert+="', '";
    sql_insert+=pwd;
    sql_insert+= "')";
    if(users.find(name) == users.end()){
            m_mutex.lock();
            LOG_INFO("sql insert: "+sql_insert);

            m_mysql = Sql_pool::getsql()->get_one();
            int res = mysql_query(m_mysql,sql_insert.c_str());
            Sql_pool::getsql()->push_one(m_mysql);

            users[name] = pwd;
            m_mutex.unlock();

            if(!res){
                code->set_errcode(0);
                code->set_errmsg("");
                response->set_sucess(true);
            }else{
                code->set_errcode(2);
                code->set_errmsg("sql error");
                response->set_sucess(false);
            }
        }else{
            code->set_errcode(1);
            code->set_errmsg("already have you");
            response->set_sucess(false);
        }

}

std::mutex my_service::m_mutex;