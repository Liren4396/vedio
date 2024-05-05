#ifndef __MY_DATA__
#define __MY_DATA__
#include "util.hpp"
#include <cstdlib>
#include <mutex>
#include <mysql/mysql.h>

namespace aod{
#define HOST "localhost"
#define USER "root"
#define PASS "comp9313"
#define NAME "aod_system"
    static MYSQL *MysqlInit() {
        MYSQL *mysql =  mysql_init(NULL);
        if (mysql == NULL) {
            fprintf(stderr, "mysql_init() failed\n");
            return NULL;
        }
        if (mysql_real_connect(mysql, HOST, USER, PASS, NAME, 0, NULL, 0) == NULL) {
            fprintf(stderr, "connect mysql server failed: %s\n", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }
    static void MysqlDestroy(MYSQL *mysql) {
        if (mysql != NULL) {
            mysql_close(mysql);
        }
        return;
    }
    static bool Mysql_Query(MYSQL *mysql, const std::string &sql) {
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0) {
            std::cout << "query sql failed" << std::endl;
            std::cout << mysql_error(mysql) << std::endl;
            return false;
        }
        return true;
    }
    class TableVideo{
        private:
            MYSQL *_mysql;
            std::mutex _mutex;
        public:
            TableVideo() {
                _mysql = MysqlInit();
                if (_mysql == NULL) {
                    exit(-1);
                }
            }
            ~TableVideo() {
                MysqlDestroy(_mysql);
            }
            bool Insert(const Json::Value &video) {
                // id name info vedio image
                std::string sql;
                sql.resize(4096, video["info"].asString().size()); // avoid info too long
                #define INSERT_VIDEO "insert tb_video values(null, '%s', '%s', '%s', '%s');"
                if (video["name"].asString().size() == 0) {
                    return false;
                }
                sprintf(&sql[0], INSERT_VIDEO, video["name"].asCString(), 
                video["info"].asCString(), 
                video["video"].asCString(),
                video["image"].asCString());
                return Mysql_Query(_mysql, sql);
            }

            bool Update(int video_id, const Json::Value &video) {
                std::string sql;
                sql.resize(4096 + video["info"].asString().size());
                #define UPDATE_VIDEO "update tb_video set name='%s', info='%s' where id=%d;"
                sprintf(&sql[0], UPDATE_VIDEO, video["name"].asCString(), 
                video["info"].asCString(), video_id);
                return Mysql_Query(_mysql, sql);
            }
            bool Delete(int video_id) {
                #define DELETE_VIDEO "delete from tb_video where id=%d;"
                char sql[1024] = {0};
                sprintf(sql, DELETE_VIDEO, video_id);
                return Mysql_Query(_mysql, sql);
            }

            bool SelectAll(Json::Value *videos) {
                #define SELECTALL_VIDEO "select * from tb_video;"
                _mutex.lock();
                bool ret = Mysql_Query(_mysql, SELECTALL_VIDEO);
                if (ret == false) {
                    _mutex.unlock();
                    return false;
                }
                MYSQL_RES *res = mysql_store_result(_mysql);
                if (res == NULL) {
                    std::cout << "mysql store result failed\n";
                    _mutex.unlock();
                    return false;
                }
                _mutex.unlock();
                int num_rows = mysql_num_rows(res);
                for (int i = 0; i < num_rows; i++) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    Json::Value video;
                    video["id"] = row[0];
                    video["name"] = row[1];
                    video["info"] = row[2];
                    video["video"] = row[3];
                    video["image"] = row[4];
                    videos->append(video);
                }
                mysql_free_result(res);
                return true;
            }
            bool SelectOne(int video_id, Json::Value *video) {
                #define SELECTONE_VIDEO "select * from tb_video where id=%d;"
                char sql[1024] = {0};
                sprintf(sql, SELECTONE_VIDEO, video_id);
                _mutex.lock();
                bool ret = Mysql_Query(_mysql, sql);
                if (ret == false) {
                    _mutex.unlock();
                    return false;
                }
                MYSQL_RES *res = mysql_store_result(_mysql);
                if (res == NULL) {
                    std::cout << "mysql store result failed\n";
                    _mutex.unlock();
                    return false;
                }
                _mutex.unlock();
                int num_rows = mysql_num_rows(res);
                if (num_rows != 1) {
                    std::cout << "have no data!\n";
                    mysql_free_result(res);
                    return false;
                }
                MYSQL_ROW row = mysql_fetch_row(res);
                (*video)["id"] = video_id;
                (*video)["name"] = row[1];
                (*video)["info"] = row[2];
                (*video)["video"] = row[3];
                (*video)["image"] = row[4];
                mysql_free_result(res);
                return true;
            }
            bool SelectLike(const std::string &key, Json::Value *videos) {
                
                #define SELECTLIKE_VIDEO "select * from tb_video where name like '%%%s%%';"
                
                char sql[1024] = {0};
                
                sprintf(sql, SELECTLIKE_VIDEO, key.c_str());
                _mutex.lock();
                bool ret = Mysql_Query(_mysql, sql);
                if (ret == false) {
                    _mutex.unlock();
                    return false;
                }
                MYSQL_RES *res = mysql_store_result(_mysql);
                if (res == NULL) {
                    std::cout << "mysql store result failed\n";
                    _mutex.unlock();
                    return false;
                }
                _mutex.unlock();
                
                int num_rows = mysql_num_rows(res);
                for (int i = 0; i < num_rows; i++) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    Json::Value video;
                    printf("1\n");
                    video["id"] = atoi(row[0]);
                    
                    video["name"] = row[1];
                    video["info"] = row[2];
                    video["video"] = row[3];
                    video["image"] = row[4];
                    
                    videos->append(video);
                }
                
                mysql_free_result(res);
                return true;
            }
    };
}

#endif