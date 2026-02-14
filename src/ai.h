// Nanami deskpet
// Copyright (C) 2026  CorvusCinereus
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
