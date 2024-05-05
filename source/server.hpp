#include "data.hpp"
#include "httplib.h"

namespace aod {
#define WWWROOT "./www"
#define VIDEO_ROOT "/video/"
#define IMAGE_ROOT "/image/"
    TableVideo *tb_video = NULL;

    class Server {

        private:
            int _port;
            httplib::Server  _svr;
        private:
            static void Insert(const httplib::Request &req, httplib::Response &rsp) {
                if (req.has_file("name") == false ||
                    req.has_file("info") == false ||
                    req.has_file("video") == false ||
                    req.has_file("image") == false) {
                        rsp.status = 400;
                        rsp.body = R"({"result":false, "reason":"upload data info error"})";
                        rsp.set_header("Content-Type", "application/json");
                        return ;
                    }
                
                httplib::MultipartFormData name = req.get_file_value("name"); // name
                httplib::MultipartFormData info = req.get_file_value("info"); // info
                httplib::MultipartFormData video = req.get_file_value("video"); // video file
                httplib::MultipartFormData image = req.get_file_value("image"); // image file

                std::string video_name = name.content;
                std::string video_info = info.content;
                std::string root = WWWROOT;
                std::string video_path = root + VIDEO_ROOT + video_name + video.filename;
                std::string image_path = root + IMAGE_ROOT + video_name + image.filename;

                if (FileUtil(video_path).SetContent(video.content) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"video file save failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }

                if (FileUtil(image_path).SetContent(image.content) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"image file save failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }

                Json::Value video_json;
                video_json["name"] = video_name;
                video_json["info"] = video_info;
                video_json["video"] = VIDEO_ROOT + video_name + video.filename;
                video_json["image"] = IMAGE_ROOT + video_name + image.filename;
                if (tb_video->Insert(video_json) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"database add failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                rsp.set_redirect("/index.html", 303);
                return;
            }
            static void Update(const httplib::Request &req, httplib::Response &rsp) {
                int video_id = stoi(req.matches[1]);
                Json::Value video;
                if (JsonUtil::UnSerialize(req.body, &video) == false) {
                    rsp.status = 400;
                    rsp.body = R"({"result":false, "reason":"new video info format analysis failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                if (tb_video->Update(video_id, video) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"modify database failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                return ;
            }
            static void Delete(const httplib::Request &req, httplib::Response &rsp) {
                // get deleted id
                int video_id = stoi(req.matches[1]);
                // delete video file and image fikle
                Json::Value video;
                if (tb_video->SelectOne(video_id, &video) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"video info doesnt exist"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                std::string root = WWWROOT;
                std::string video_path = root + video["video"].asString();
                std::string image_path = root + video["image"].asString();
                remove(video_path.c_str());
                remove(image_path.c_str());
                // delete database info
                if (tb_video->Delete(video_id) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"delete database info failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                
                return ;
            }
            static void SelectOne(const httplib::Request &req, httplib::Response &rsp) {
                // 1.get video id
                // 2.search specified video info
                Json::Value video;
                int video_id = stoi(req.matches[1]);
                if (tb_video->SelectOne(video_id, &video) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"search database's specified video info failed"})";
                    rsp.set_header("Content-Type", "application/json");
                    return ;
                }
                

                // 3. organize response content --- json format string
                JsonUtil::Serialize(video, &rsp.body);
                rsp.set_header("Content-Type", "application/json");
                return ;
            }
            static void SelectAll(const httplib::Request &req, httplib::Response &rsp) {
                //  /video  & /video?search="key word"
                bool select_flag = true;
                std::string search_key;
                if (req.has_param("search") == true) {
                    select_flag = false; // like search
                    search_key = req.get_param_value("search");
                }
                Json::Value videos;
                if (select_flag == true) {
                    if (tb_video->SelectAll(&videos) == false) {
                        rsp.status = 500;
                        rsp.body = R"({"result":false, "reason":"search database's all videos info failed"})";
                        rsp.set_header("Content-Type", "application/json");
                        return ;
                    }
                } else {
                    if (tb_video->SelectLike(search_key, &videos) == false) {
                        rsp.status = 500;
                        rsp.body = R"({"result":false, "reason":"search database's matched video info failed"})";
                        rsp.set_header("Content-Type", "application/json");
                        return ;
                    }
                }
                JsonUtil::Serialize(videos, &rsp.body);
                rsp.set_header("Content-Type", "application/json");
                return ;
            }
            static void SearchVideos(const httplib::Request &req, httplib::Response &rsp) {
                if(!req.has_param("query")) {
                    rsp.status = 400;
                    rsp.body = R"({"result":false, "reason":"No search query provided"})";
                    rsp.set_header("Content-Type", "application/json");
                    return;
                }
                std::string query = req.get_param_value("query");
                Json::Value videos;
                if(tb_video->SelectLike(query, &videos) == false) {
                    rsp.status = 500;
                    rsp.body = R"({"result":false, "reason":"Search operation failed"})";
                    rsp.set_header("Content-Type", "application/json");
                } else {
                    JsonUtil::Serialize(videos, &rsp.body);
                    rsp.set_header("Content-Type", "application/json");
                }
                return;
            }
        public:
            Server(int port): _port(port) {}
            bool RunModule() {
                tb_video = new TableVideo();
                FileUtil(WWWROOT).CreateDirectory();
                std::string root = WWWROOT;
                std::string video_real_path = root + VIDEO_ROOT;
                FileUtil(video_real_path).CreateDirectory();
                std::string image_real_path = root + IMAGE_ROOT;
                FileUtil(image_real_path).CreateDirectory();

                // establish http server, run
                // 1.set static root dir
                _svr.set_mount_point("/", root);
                // 2.add request -- deal relation function
                _svr.Post("/video", Insert);
                _svr.Delete("/video/(\\d+)", Delete);
                _svr.Put("/video/(\\d+)", Update);
                _svr.Get("/video/(\\d+)", SelectOne);
                _svr.Get("/video", SelectAll);
                _svr.Get("/search", SearchVideos);
                // start server 
                _svr.listen("0.0.0.0", _port);
                return true;
            }
    };
}