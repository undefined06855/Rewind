#pragma once
#include <Geode/modify/UILayer.hpp>
#include "../RewindButton.hpp"

class $modify(HookedUILayer, UILayer) {
    struct Fields {
        RewindButton* m_rewindButton;
        Fields();
    };

    bool init(GJBaseGameLayer* gjbgl);
    void updateCheckState();
};
