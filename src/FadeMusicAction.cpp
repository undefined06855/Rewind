#include "FadeMusicAction.hpp"
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/fmod/fmod_errors.h>

FadeMusicAction::FadeMusicAction()
    : m_dir(FadeMusicDirection::FadeOut) {}

FadeMusicAction* FadeMusicAction::create(float d, FadeMusicDirection dir) {
    auto ret = new FadeMusicAction;
    if (ret->init(d, dir)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool FadeMusicAction::init(float d, FadeMusicDirection dir) {
    if (!CCActionInterval::initWithDuration(d)) return false;

    m_dir = dir;
    
    return true;
}

void FadeMusicAction::update(float time) {
    // circular ease, taken from fade music on death
    float eased;
    if (m_dir == FadeMusicDirection::FadeIn) {
        eased = std::sqrt(1.f - std::pow(time - 1.f, 2.f));
    } else {
        eased = 1.f - std::sqrt(1.f - std::pow(time - 1.f, 2.f));
    }

    auto engine = FMODAudioEngine::get();
    engine->m_backgroundMusicChannel->setPitch(eased); // music
    engine->m_globalChannel->setPitch(eased); // sfx
}
