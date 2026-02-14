add_rules("mode.debug", "mode.release")
add_requires("raylib", "rlimgui", "libcurl")

if is_plat("linux") then
    add_requires("libx11")
end

target("Nanami")
    set_kind("binary")
    set_languages("c++20")
    add_files("src/*.cpp")
    add_packages("raylib", "rlimgui", "libcurl")
    if is_plat("linux") then
        add_packages("libx11")
    end