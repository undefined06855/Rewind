#pragma once
#include <Geode/modify/CCRenderTexture.hpp>

class $modify(HookedCCRenderTexture, cocos2d::CCRenderTexture) {
    static void onModify(auto& self);
    
    void begin();
    void end();
};
