#include "server.hpp"

void FileTest() {
    aod::FileUtil("./www").CreateDirectory();
    aod::FileUtil("./www/index.html").SetContent("<html></html>");
    std::string body;
    aod::FileUtil("./www/index.html").GetContent(&body);
    std::cout << body << std::endl;
    std::cout << aod::FileUtil("./www/index.html").Size() << std::endl;
}
void JsonTest() {
    Json::Value val;
    val["name"] = "xiaozhang";
    val["age"] = 18;
    val["sex"] = "male";
    val["score"].append(77.5);
    val["score"].append(87.5);
    val["score"].append(97.5);

    std::string body;
    aod::JsonUtil::Serialize(val, &body);
    std::cout << body << std::endl;

    Json::Value stu;
    aod::JsonUtil::UnSerialize(body, &stu);
    std::cout << stu["name"].asString() << std::endl;
    std::cout << stu["age"].asString() << std::endl;
    std::cout << stu["sex"].asString() << std::endl;
    for (auto &a : stu["score"]) {
        std::cout << a.asFloat() << std::endl;
    }
}
void ServerTest() {
    aod::Server server(8080);
    server.RunModule();
}
void DataTest() {
    ServerTest();
    //aod::TableVideo tb_video;
    //Json::Value video;
    //video["name"] = "second video";
    //video["info"] = "modified second information";
    //video["video"] = "/video/transformer.mp4";
    //video["image"] = "/image/transformer.jpg";
    //tb_video.Insert(video);
    //tb_video.Update(2, video);
    //tb_video.SelectAll(&video);
    //tb_video.SelectOne(1, &video);

    //tb_video.SelectLike("f", &video);
    //std::string body;
    //aod::JsonUtil::Serialize(video, &body);
    //std::cout << body << std::endl;
    //tb_video.Delete(1);

}

int main() {
    //FileTest();
    //JsonTest();
    printf("0");
    DataTest();
    return 0;
}