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

#ifndef _NANAMI_H_
#define _NANAMI_H_

#include <imgui.h>
#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include "ai.h"

namespace cc {

class Nanami {
    enum Status : unsigned char {
        Normal, SpeakOrSing
    };

public:
    Nanami();
    ~Nanami();

    void run();

private:
    float m_Scale;
    double m_RestTime;
    double m_LastRest;
    unsigned char m_MusicMod;
    bool m_IsDragging;
    bool m_EnableAI;
    bool m_CursorFollow;
    bool m_DrawMenu;
    bool m_DrawMusicMenu;
    bool m_Chatting;
    std::vector<std::string> m_Musics;
    std::unique_ptr<cc::AI> m_AI;

    Status m_Status;

    Texture *m_Texture;
    Texture m_Images[7];
    Music m_Music;
    ImFont *m_Font;

    void cursor_follow();
    void draw_menu();
    void draw_music_menu();
    void chat();
};

}

#endif // !_NANAMI_H_
