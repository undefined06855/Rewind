#pragma once
#include <Geode/modify/PlayLayer.hpp>

class $modify(HookedPlayLayer, PlayLayer) {
    void togglePracticeMode(bool enabled);
    void pauseGame(bool becauseOfSuspend);
};
