#pragma once
#include <Geode/modify/PlayLayer.hpp>

class $modify(HookedPlayLayer, PlayLayer) {
    struct Fields {
        cocos2d::CCLabelBMFont* m_practiceInfoLabel;

        Fields();
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects);
    void togglePracticeMode(bool enabled);
    void pauseGame(bool becauseOfSuspend);
};
