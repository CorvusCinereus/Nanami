#include "ai.h"
#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

namespace cc {

AI::AI() : m_History("") {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_Curl = curl_easy_init();

    std::ifstream file("ai.json");
    if (file.is_open()) {
        std::string buffer, data;
        while (std::getline(file, buffer)) {
            data += buffer;
        }
        
        try {
            m_Config = nlohmann::json::parse(data);
        } catch (nlohmann::json::parse_error& e) {
            std::cerr << e.what() << std::endl;
            curl_easy_cleanup(m_Curl);
            exit(1);
        }
        file.close();
    }

    std::ifstream aemeath("aemeath.txt");
    if (aemeath.is_open()) {
        std::string buffer;
        while (std::getline(aemeath, buffer)) {
            m_History += buffer;
        }
        aemeath.clear();
    }
}

AI::~AI() {
    if (m_Curl) curl_easy_cleanup(m_Curl);
    curl_global_cleanup();
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void AI::chat(std::string& respone, std::string input) {
    respone.clear();
    if (m_Curl) {
        std::string url = m_Config["url"];
        std::string model = m_Config["model"];
        std::string responeBuffer;
        
        curl_easy_setopt(m_Curl, CURLOPT_URL, url.c_str());
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);

        input.erase(std::remove_if(input.begin(), input.end(), [](char c) {return c == ' ' || c == '\n';}), input.end());
        m_History += ';' + input;
        std::string postData = "{\"input\":\"" + m_History;
        postData += "\",\"model\":\"";
        postData += model + "\"}";

        curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, postData.c_str());

        curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &responeBuffer);
        m_Res = curl_easy_perform(m_Curl);

        if (m_Res != CURLE_OK) {
            respone.clear();
            respone = curl_easy_strerror(m_Res);
        } else {
            respone.clear();
            nlohmann::json json = nlohmann::json::parse(responeBuffer);
            respone += json["output"][1]["content"];
        }

        curl_slist_free_all(headers);
    } else {
        respone = "Error: Cannot init curl.";
    }
}

}
