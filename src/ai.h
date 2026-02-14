#ifndef _AI_H_
#define _AI_H_
#include <string>
#include "json.hpp"

namespace cc {
#include <curl/curl.h>

class AI {
public:
    AI();
    ~AI();

    void chat(std::string& respone, std::string input);

private:
    CURL *m_Curl;
    CURLcode m_Res;
    nlohmann::json m_Config;
    std::string m_History;
};

}

#endif // !_AI_H_
