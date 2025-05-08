#include "UILayer.hpp"

HookedUILayer::Fields::Fields()
    : m_rewindButton(nullptr) {}

bool HookedUILayer::init(GJBaseGameLayer* gjbgl) {
    if (!UILayer::init(gjbgl)) return false;

    // these x positions took like 20 minutes to figure out so that it's centred
    // would be nice to be able to put these in updateCheckState but node ids
    // aren't on the nodes yet and they're not stored in any members

    auto rewindButton = RewindButton::create();
    rewindButton->setOpacity(GameManager::get()->m_practiceOpacity * 255);
    rewindButton->setPositionX(13.25f);
    m_checkpointMenu->addChild(rewindButton);
    m_fields->m_rewindButton = rewindButton;

    auto addBtn = m_checkpointMenu->getChildByID("add-checkpoint-button");
    if (addBtn) addBtn->setPositionX(-58.5f);
    
    auto removeBtn = m_checkpointMenu->getChildByID("remove-checkpoint-button");
    if (removeBtn) removeBtn->setPositionX(71.75f);

    return true;
}

void HookedUILayer::updateCheckState() {
    UILayer::updateCheckState();
    auto fields = m_fields.self();
    
    if (!fields->m_rewindButton) return;
    fields->m_rewindButton->setOpacity(GameManager::get()->m_practiceOpacity * 255);
}
