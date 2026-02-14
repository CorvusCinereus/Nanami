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

#include "nanami.h"
#include "json.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1080

using namespace cc;

extern void getGlobalMousePosition(float& x, float& y);

Nanami::Nanami() : m_Scale(0.7), m_IsDragging(false), m_CursorFollow(false), m_Status(SpeakOrSing), m_DrawMenu(false), m_DrawMusicMenu(false), m_RestTime(1800.0), m_LastRest(0.0), m_EnableAI(false), m_Chatting(false), m_MusicMod(0) {
    std::string data = "";
    nlohmann::json json;

    // 加载配置文件
    std::ifstream configFile("settings.json");
    if (configFile.is_open()) {
        std::string buffer;
        while (std::getline(configFile, buffer)) {
            data += buffer;
        }

        try {
            json = nlohmann::json::parse(data);
            m_Scale = json["scale"];
            m_RestTime = json["restTime"];
            m_RestTime *= 60.0;
            m_EnableAI = json["enableAI"];
        } catch (nlohmann::json::parse_error& e) {
            std::cerr << e.what() << std::endl;
            std::exit(1);
        }
        
        configFile.close();
    } else {
        std::exit(1);
    }

    // 初始化raylib
    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED);
    InitWindow(WINDOW_WIDTH * m_Scale, WINDOW_HEIGHT * m_Scale, "Nanami");
    InitAudioDevice();

    SetTargetFPS(30);
    SetExitKey(KEY_ESCAPE);

    // 初始化imgui
    rlImGuiSetup(false);
        ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    m_Font = io.Fonts->AddFontFromFileTTF("assets/wqy-zenhei.ttc");
    IM_ASSERT(m_Font != nullptr);
    io.FontGlobalScale = json["font"];

    // 加载资源
    Image imageBuffer;

    {  // icon
        imageBuffer = LoadImage("assets/icon.png");
        SetWindowIcon(imageBuffer);
        UnloadImage(imageBuffer);
    }

    { // images
        for (size_t i = 0; i < 7; ++i) {
            imageBuffer = LoadImage(std::format("assets/nanami{}.png", i + 1).c_str());
            m_Images[i] = LoadTextureFromImage(imageBuffer);
            UnloadImage(imageBuffer);
        }
    }
    m_Texture = &m_Images[0];

    { // musics
        for (const auto& entry : std::filesystem::directory_iterator("assets/musics")) {
            if (entry.is_regular_file()) {
                m_Musics.push_back(entry.path().string());
            }
        }
    }

    if (m_EnableAI)
        m_AI = std::make_unique<cc::AI>();

    m_Music = LoadMusicStream("assets/voice/greet.wav");
    PlayMusicStream(m_Music);

    SetWindowPosition(GetMonitorWidth(0) - GetScreenWidth(), GetMonitorHeight(0) - GetScreenHeight());
}

Nanami::~Nanami() {
    for (size_t i = 0; i < 7; ++i) {
        UnloadTexture(m_Images[i]);
    }

    if (IsMusicStreamPlaying(m_Music)) {
        StopMusicStream(m_Music);
        UnloadMusicStream(m_Music);
    }

    rlImGuiShutdown();

    CloseAudioDevice();
    CloseWindow();
}

void Nanami::run() {
    float mousePosX, mousePosY;
    Vector2 dragOffset = {0, 0};
    bool reset = false;

    while (!WindowShouldClose()) {
        getGlobalMousePosition(mousePosX, mousePosY);

        // 拖拽
        if (!m_DrawMenu && !m_Chatting && CheckCollisionPointRec(GetMousePosition(), {0, 0, WINDOW_WIDTH * m_Scale, WINDOW_HEIGHT * m_Scale})) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                m_IsDragging = true;
                dragOffset.x = mousePosX;
                dragOffset.y = mousePosY;
            }
        }

        if (m_IsDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Vector2 winPos = GetWindowPosition();
                Vector2 newPos = {
                    winPos.x + mousePosX - dragOffset.x,
                    winPos.y + mousePosY - dragOffset.y
                };
                SetWindowPosition(newPos.x, newPos.y);
                dragOffset.x = mousePosX;
                dragOffset.y = mousePosY;
            } else {
                SetMouseCursor(MOUSE_CURSOR_ARROW);
                m_IsDragging = false;
            }
        }

        // 鼠标跟随
        if (IsKeyPressed(KEY_BACKSPACE) && !m_Chatting) {
            m_CursorFollow = !m_CursorFollow;
            if (m_CursorFollow) {
                m_Scale /= 3.0;
                SetWindowSize(720 * m_Scale, 584 * m_Scale);
                m_Texture = &m_Images[4];
            } else {
                m_Scale *= 3.0;
                SetWindowSize(WINDOW_WIDTH * m_Scale, WINDOW_HEIGHT * m_Scale);
                m_Texture = &m_Images[0];
            }
        }

        if (m_CursorFollow)
            SetWindowPosition(mousePosX + 10, mousePosY + 10);

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            m_DrawMenu = !m_DrawMenu;

        if (std::fabs(m_RestTime - GetTime() + m_LastRest) < 0.1) {
            if (IsMusicStreamPlaying(m_Music)) {
                StopMusicStream(m_Music);
                UnloadMusicStream(m_Music);
            }
            m_Music = LoadMusicStream("assets/voice/rest.wav");
            PlayMusicStream(m_Music);
            m_Status = SpeakOrSing;
            reset = true;
            m_Texture = &m_Images[2];
            if (m_CursorFollow) {
                m_CursorFollow = false;
                m_Scale *= 3.0;
                SetWindowSize(WINDOW_WIDTH * m_Scale, WINDOW_HEIGHT * m_Scale);
            }
        }

        // 绘制
        BeginDrawing();

        ClearBackground(BLANK);

        switch (m_Status) {
            case Normal:
                break;
            case SpeakOrSing:
                UpdateMusicStream(m_Music);
                if (std::fabs(GetMusicTimeLength(m_Music) - GetMusicTimePlayed(m_Music)) < 0.2) {
                    StopMusicStream(m_Music);
                    UnloadMusicStream(m_Music);

                    if (reset) {
                        m_LastRest = GetTime();
                        reset = false;
                    }
                    m_Status = Normal;
                }
                break;
            default:
                break;
        }

        if (m_CursorFollow)
            cursor_follow();

        DrawTextureEx(*m_Texture, {0, 0}, 0.0, m_Scale, WHITE);
        if (m_DrawMenu || m_Chatting) {
            rlImGuiBegin();
            ImGui::PushFont(m_Font);

            if (m_DrawMusicMenu)
                draw_music_menu();

            if (m_DrawMenu && !m_DrawMusicMenu)
                draw_menu();

            if (m_Chatting) chat();

            ImGui::PopFont();
            rlImGuiEnd();
        }

        EndDrawing();
    }
}

void Nanami::cursor_follow() {
    static bool front = true;
    static unsigned char count = 0;
    if (count != 4) {
        ++count;
    } else {
        m_Texture = front ? &m_Images[4] : &m_Images[5];
        front = !front;
        count = 0;
    }
}

void Nanami::draw_menu() {
    ImGui::Begin("菜单");

    if (m_Status != SpeakOrSing && ImGui::Button("互动")) {
        m_Status = SpeakOrSing;
        m_DrawMenu = false;

        int index = GetRandomValue(1, 8);
        switch (index) {
        case 2:
            m_Texture = &m_Images[3];
            break;
        case 4:
        case 8:
            m_Texture = &m_Images[6];
            break;
        case 5:
        case 1:
            m_Texture = &m_Images[1];
            break;
        case 7:
        case 6:
            m_Texture = &m_Images[2];
            break;
        default:
            m_Texture = &m_Images[0];
        }
        m_Music = LoadMusicStream(std::format("assets/voice/{}.wav", index).c_str());
        PlayMusicStream(m_Music);
    }

    if (m_Status != SpeakOrSing && ImGui::Button("音乐")) {
        m_DrawMusicMenu = true;
    }

    if (m_EnableAI) {
        if (!m_Chatting && ImGui::Button("对话")) {
            m_Chatting = true;
            m_DrawMenu = false;
        } else if (m_Chatting && ImGui::Button("结束对话")) {
            m_Chatting = false;
            m_DrawMenu = false;
        }
    }

    ImGui::End();
}

void Nanami::draw_music_menu() {
    ImGui::Begin("播放模式");

    if (!m_MusicMod) {
        if (ImGui::Button("随机")) {
            m_MusicMod = 0;
            m_Status = SpeakOrSing;
            m_DrawMusicMenu = false;
            m_DrawMenu = false;

            m_Music = LoadMusicStream(m_Musics[GetRandomValue(0, m_Musics.size() - 1)].c_str());
            PlayMusicStream(m_Music);
        }

        if (ImGui::Button("列表")) {
            m_MusicMod = 1;
        }
    } else {
        for (const auto& i : m_Musics) {
            if (ImGui::Button(i.substr(14).c_str())) {
                m_Music = LoadMusicStream(i.c_str());
                PlayMusicStream(m_Music);
                m_MusicMod = 0;
                m_DrawMenu = false;
                m_DrawMusicMenu = false;
                m_Status = SpeakOrSing;
            }
        }
    }

    if (ImGui::Button("取消")) {
        m_DrawMusicMenu = false;
    }

    ImGui::End();
}

void Nanami::chat() {
    static std::string respone = "";

    ImGui::Begin("对话");

    static char buffer[2048] = {0};
    static std::future<void> fut;
    static bool waiting = false;

    if (waiting && fut.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        fut.get();
        waiting = false;
    }

    if (respone.empty())
        ImGui::InputTextMultiline("", buffer, IM_ARRAYSIZE(buffer));
    else if (waiting)
        ImGui::Text("等待回复中...");
    else
        ImGui::TextWrapped(respone.c_str());

    if (respone.empty() && !waiting && ImGui::Button("发送")) {
        fut = std::async(std::launch::async, [this] {
                m_AI->chat(respone, buffer);
                });
        waiting = true;
    } else if (!respone.empty() && ImGui::Button("继续对话")) {
        respone.clear();
        memset(buffer, 0, sizeof(buffer));
    }

    ImGui::End();
}
