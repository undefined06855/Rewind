#pragma once
#include <Geode/modify/CCRenderTexture.hpp>

class $modify(HookedCCRenderTexture, cocos2d::CCRenderTexture) {
    void begin();
    void end();
};
