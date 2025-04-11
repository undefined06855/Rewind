#pragma once
#include <Geode/cocos/actions/CCActionInterval.h>

enum class FadeMusicDirection {
    FadeIn, FadeOut
};

class FadeMusicAction : public cocos2d::CCActionInterval {
    FadeMusicAction();
public:
    static FadeMusicAction* create(float d, FadeMusicDirection dir);
    bool init(float d, FadeMusicDirection dir);

    FadeMusicDirection m_dir;

    void update(float time);
};
