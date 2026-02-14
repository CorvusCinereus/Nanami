#include "nanami.h"
#include "json.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1080

using namespace cc;

extern void getGlobalMousePosition(float& x, float& y);

Nanami::Nanami() : m_Scale(0.7), m_IsDragging(false), m_CursorFollow(false), m_Status(Normal), m_DrawMenu(false), m_DrawMusicMenu(false) {
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
        } catch (nlohmann::json::parse_error& e) {
            std::cerr << e.what() << std::endl;
            std::exit(1);
        }
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
                m_Musics.push_back(entry.path().string().c_str());
            }
        }
    }

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

    while (!WindowShouldClose()) {
        getGlobalMousePosition(mousePosX, mousePosY);

        // 拖拽
        if (!m_DrawMenu && CheckCollisionPointRec(GetMousePosition(), {0, 0, WINDOW_WIDTH * m_Scale, WINDOW_HEIGHT * m_Scale})) {
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
        if (IsKeyPressed(KEY_BACKSPACE)) {
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
                    m_Status = Normal;
                }
                break;
            default:
                break;
        }

        if (m_CursorFollow)
            cursor_follow();

        DrawTextureEx(*m_Texture, {0, 0}, 0.0, m_Scale, WHITE);
        if (m_DrawMenu) {
            rlImGuiBegin();
            ImGui::PushFont(m_Font);

            if (m_DrawMusicMenu) {
                draw_music_menu();
            } else {
                draw_menu();
            }

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

        int index = 2;// GetRandomValue(1, 7);
        switch (index) {
        case 2:
            m_Texture = &m_Images[3];
            break;
        case 4:
            m_Texture = &m_Images[6];
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

    if (ImGui::Button("对话")) {
        // TODO
    }

    ImGui::End();
}

void Nanami::draw_music_menu() {
    ImGui::Begin("播放模式");

    if (ImGui::Button("随机")) {
        m_MusicMod = 0;
        m_Status = SpeakOrSing;
        m_DrawMusicMenu = false;
        m_DrawMenu = false;

        m_Music = LoadMusicStream(m_Musics[GetRandomValue(0, m_Musics.size() - 1)].c_str());
        PlayMusicStream(m_Music);
    }

    ImGui::End();
}
