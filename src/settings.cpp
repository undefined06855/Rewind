#include <Geode/loader/SettingV3.hpp>
#include <Geode/cocos/platform/win32/CCGL.h>
#include "hooks/GJBaseGameLayer.hpp"

// shouldnt be necessary but i bet some people will be changing their mod settings
// during gameplay somehow
$on_mod(Loaded) {
    geode::listenForAllSettingChangesV3([](std::shared_ptr<geode::SettingV3> setting){
        auto gjbgl = static_cast<HookedGJBaseGameLayer*>(GJBaseGameLayer::get());
        if (!gjbgl) return;

        auto fields = gjbgl->m_fields.self();
        
        if (setting->getKey() == "frames-per-second") {
            fields->m_secondsPerFrame = 1.f / geode::Mod::get()->getSettingValue<int64_t>("frames-per-second");
        } else if (setting->getKey() == "history-length") {
            fields->m_historyLength = geode::Mod::get()->getSettingValue<int64_t>("history-length");
        } else if (setting->getKey() == "resolution-multiplier") {
            fields->m_resolutionMultiplier = geode::Mod::get()->getSettingValue<double>("resolution-multiplier");
        }
    });


    // populate defaults by getting amount of system vram
    bool hasSetRecommended = geode::Mod::get()->getSavedValue<bool>("has-set-recommended", false);
    if (hasSetRecommended) return;

    geode::Mod::get()->setSavedValue<bool>("has-set-recommended", true);

    // https://stackoverflow.com/a/5695427
    // some pretty sketchy glgetinteger calls
    auto values = new int[4]();
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, values);
    glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, values);

    if (values[0] == 0) {
        geode::log::info("No data returned from vram checks :( ({}, {}, {}, {})", values[0], values[1], values[2], values[3]);
        delete[] values;
        return;
    }

    geode::log::info("Video memory checks returned {}", values[0]);

    delete[] values;
}
