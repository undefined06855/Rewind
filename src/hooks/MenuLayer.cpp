#ifdef GEODE_IS_WINDOWS
#include "MenuLayer.hpp"
#include <Geode/cocos/platform/third_party/win32/OGLES/GL/glew.h>

// populate defaults by getting amount of system vram
bool HookedMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    auto mod = geode::Mod::get();

    bool hasSetRecommended = mod->getSavedValue<bool>("has-set-recommended", false);
    if (hasSetRecommended) return true;
    mod->setSavedValue<bool>("has-set-recommended", true);

    // https://stackoverflow.com/a/5695427
    // no intel support very sad
    int values[4] = {};
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, values); // nvidia
    glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, values); // amd (ati)

    if (values[0] == 0) {
        geode::log::info("No data returned from vram checks :( ({}, {}, {}, {})", values[0], values[1], values[2], values[3]);
        return true;
    }

    int kb = values[0];
    int frames = (kb / 40000) + 20; // https://www.desmos.com/calculator/m2hlel1fjj

    geode::log::info("Video memory checks returned {}kb (approx), mapped to {} frames", kb, frames);

    frames = std::max(20, std::min(frames, 500));

    mod->setSettingValue<int64_t>("history-length", frames);

    auto pop = FLAlertLayer::create(
        "Rewind",
        fmt::format(
            "Rewind has automagically detected {}GB of video memory free, and "
            "has set the max history length to {}, which can be customised in "
            "settings.",
            kb / 1000000, frames
        ).c_str(),
        "ok"
    );
    pop->m_scene = this;
    pop->show();

    return true;
}
#endif
