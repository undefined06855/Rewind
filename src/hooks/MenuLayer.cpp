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
        
        // this code is amazing
        auto pop = FLAlertLayer::create(
            "Rewind",
            fmt::format(
                "Rewind cannot detect the amount of video memory free"
#ifdef GEODE_IS_WINDOWS
                ", most likely because you have an <cl>Intel</c> graphics card, "
                "or are using <co>integrated graphics</c>. "
#else
                " on this platform. "
#endif
                "Although Rewind has applied <cy>suitable defaults</c>, if you"
                "know you have a <cr>bad "
                GEODE_DESKTOP("computer")
                GEODE_MOBILE("phone")
                "</c>, please adjust the history and capture frequency in the",
                "<co>mod settings</c>."
            ).c_str(),
            "ok"
        );
        pop->m_scene = this;
        pop->show();

        return true;
    }

    int kb = values[0];
    int frames = (kb / 28000) + 50; // https://www.desmos.com/calculator/lqpimloksu

    geode::log::info("Video memory checks returned {}kb (approx), mapped to {} frames", kb, frames);

    frames = std::max(20, std::min(frames, 500)); 

    mod->setSettingValue<int64_t>("history-length", frames);

    int gb = kb / 1000000;

    auto pop = FLAlertLayer::create(
        "Rewind",
        fmt::format(
            "Rewind has automagically detected <cy>{}GB</c> of video memory "
            "free, and has set the <cj>max history length</c> to <cy>{}</c>, "
            "which can be customised in the <co>mod settings</c>.",
            gb < 1 ? "less than 1" : fmt::to_string(gb), frames
        ).c_str(),
        "ok"
    );
    pop->m_scene = this;
    pop->show();

    return true;
}
#endif
