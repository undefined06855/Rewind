#include "PlayLayer.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include "GJBaseGameLayer.hpp"

HookedPlayLayer::Fields::Fields()
    : m_practiceInfoLabel(nullptr) {}

bool HookedPlayLayer::init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    auto fields = m_fields.self();

    auto binds = keybinds::BindManager::get()->getBindsFor("rewind"_spr);
    std::string labelString;
    if (binds.size() == 0) labelString = "";
    else labelString = "Rewind: ";
    
    for (auto bind : binds) {
        labelString += bind->toString();
        if (!bind->isEqual(binds.back())) labelString += " or ";
    }

    fields->m_practiceInfoLabel = cocos2d::CCLabelBMFont::create(labelString.c_str(), "bigFont.fnt");
    fields->m_practiceInfoLabel->setPositionX(getContentWidth() / 2.f);
    fields->m_practiceInfoLabel->setPositionY(15.f);
    fields->m_practiceInfoLabel->setVisible(false);
    fields->m_practiceInfoLabel->setScale(.4f);
    fields->m_practiceInfoLabel->setID("practice-info-label"_spr);
    addChild(fields->m_practiceInfoLabel, 102);

    auto checkpointMenu = m_uiLayer->m_checkpointMenu;
    checkpointMenu->setPositionY(checkpointMenu->getPositionY() + 20.f);

    return true;
}

void HookedPlayLayer::togglePracticeMode(bool enabled) {
    PlayLayer::togglePracticeMode(enabled);

    auto fields = m_fields.self();

    if (enabled && !GameManager::get()->getGameVariable("0071" /* Hide Practice Buttons */)) {
        fields->m_practiceInfoLabel->setVisible(true);
    } else {
        fields->m_practiceInfoLabel->setVisible(false);
    }

    reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields->m_history.clear();
}

void HookedPlayLayer::pauseGame(bool becauseOfSuspend) {
    bool isRewinding = reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields->m_isRewinding;
    if (isRewinding) return;
    PlayLayer::pauseGame(becauseOfSuspend);
}
