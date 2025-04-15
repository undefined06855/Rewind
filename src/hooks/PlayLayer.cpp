#include "PlayLayer.hpp"
#include "GJBaseGameLayer.hpp"
#include "../RewindButton.hpp"

bool HookedPlayLayer::init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    // these x positions took like 20 minutes to figure out so that it's centred

    auto checkpointMenu = m_uiLayer->m_checkpointMenu;
    auto rewindButton = RewindButton::create();
    rewindButton->setPositionX(13.25f);
    checkpointMenu->addChild(rewindButton);

    auto addBtn = checkpointMenu->getChildByID("add-checkpoint-button");
    if (addBtn) addBtn->setPositionX(-58.5f);
    
    auto removeBtn = checkpointMenu->getChildByID("remove-checkpoint-button");
    if (removeBtn) removeBtn->setPositionX(71.75f);

    return true;
}

void HookedPlayLayer::togglePracticeMode(bool enabled) {
    PlayLayer::togglePracticeMode(enabled);
    if (!enabled) reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields->m_history.clear();
}

void HookedPlayLayer::pauseGame(bool becauseOfSuspend) {
    auto fields = reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields.self();
    if (fields->m_isRewinding || fields->m_isTransitioningOut) return;
    PlayLayer::pauseGame(becauseOfSuspend);
}
