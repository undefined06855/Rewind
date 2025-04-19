#include <Geode/loader/SettingV3.hpp>
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
}
