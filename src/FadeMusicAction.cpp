#include "FadeMusicAction.hpp"
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
    auto target = (FMODAudioEngine*)getTarget();

    if (!target /* || target->m_bFading? */) {
        target->stopAction(this);
        return;
    }

    // circular ease, taken from fade music on death
    float ease;
    if (m_dir == FadeMusicDirection::FadeIn) {
        ease = std::sqrt(1.f - std::pow(time - 1.f, 2.f));
    } else {
        ease = 1.f - std::sqrt(1.f - std::pow(time - 1.f, 2.f));
    }

    // really fucking brute force way of doing this but it does work
    for (auto& [_, channel] : target->m_channelIDToChannel) {
        channel->setPitch(ease);
    }

    // sets it but i believe each channel overrides the pitch individually
    // so setting it does nothing
    // target->m_globalChannel->setPitch(ease);
}
