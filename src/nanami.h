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
