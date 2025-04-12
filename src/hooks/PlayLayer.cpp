#include "PlayLayer.hpp"
#include "GJBaseGameLayer.hpp"
#include "../RewindButton.hpp"

bool HookedPlayLayer::init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    auto checkpointMenu = m_uiLayer->m_checkpointMenu;
    auto rewindButton = RewindButton::create();
    checkpointMenu->addChild(rewindButton);

    auto addBtn = checkpointMenu->getChildByID("add-checkpoint-button");
    if (addBtn) addBtn->setPositionX(-72.f);
    
    auto removeBtn = checkpointMenu->getChildByID("remove-checkpoint-button");
    if (removeBtn) removeBtn->setPositionX(59.f);

    return true;
}

void HookedPlayLayer::togglePracticeMode(bool enabled) {
    PlayLayer::togglePracticeMode(enabled);
    reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields->m_history.clear();
}

void HookedPlayLayer::pauseGame(bool becauseOfSuspend) {
    bool isRewinding = reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields->m_isRewinding;
    if (isRewinding) return;
    PlayLayer::pauseGame(becauseOfSuspend);
}
