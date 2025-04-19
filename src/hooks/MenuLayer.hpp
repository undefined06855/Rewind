#pragma once
#ifdef GEODE_IS_WINDOWS
#include <Geode/modify/MenuLayer.hpp>

class $modify(HookedMenuLayer, MenuLayer) {
    bool init();
};

#endif
