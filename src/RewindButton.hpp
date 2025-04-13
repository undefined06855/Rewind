#pragma once
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

class RewindButton : public CCMenuItemSpriteExtra {
public:
    static RewindButton* create();
    bool init();

    void fakeCallback(cocos2d::CCObject* sender);
    void selected();
    void unselected();
};
