#include "PlayLayer.hpp"
#include "GJBaseGameLayer.hpp"

void HookedPlayLayer::togglePracticeMode(bool enabled) {
    PlayLayer::togglePracticeMode(enabled);
    auto fields = reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields.self();
    if (!enabled && !fields->m_captureInNormalMode) fields->m_history.clear();
}

void HookedPlayLayer::pauseGame(bool becauseOfSuspend) {
    auto fields = reinterpret_cast<HookedGJBaseGameLayer*>(this)->m_fields.self();
    if (fields->m_isRewinding || fields->m_isTransitioningOut) return;
    PlayLayer::pauseGame(becauseOfSuspend);
}
