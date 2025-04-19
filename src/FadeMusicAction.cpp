#include "FadeMusicAction.hpp"
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/fmod/fmod_errors.h>

FadeMusicAction::FadeMusicAction()
    : m_dir(FadeMusicDirection::FadeOut)
    , m_initialOrTargetPitches({}) {}

FadeMusicAction* FadeMusicAction::create(float d, FadeMusicDirection dir, const std::unordered_map<int, float>& targetPitches) {
    auto ret = new FadeMusicAction;
    if (ret->init(d, dir, targetPitches)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

FadeMusicAction* FadeMusicAction::create(float d, FadeMusicDirection dir) {
    return FadeMusicAction::create(d, dir, {});
}

bool FadeMusicAction::init(float d, FadeMusicDirection dir, const std::unordered_map<int, float>& targetPitches) {
    if (!CCActionInterval::initWithDuration(d)) return false;

    m_dir = dir;

    // if direction is fade out, initialOrTarget stores initial
    // if direction is fade in, initialOrTarget stores target
    if (dir == FadeMusicDirection::FadeOut) {
        for (auto& [id, channel] : FMODAudioEngine::get()->m_channelIDToChannel) {
            m_initialOrTargetPitches[id] = 0.f;
            channel->getPitch(&m_initialOrTargetPitches[id]);
        }
    } else {
        m_initialOrTargetPitches = /* (copy) */ targetPitches;
    }

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

    // really fucking brute force way of doing this but it does work
    // multiplying by initialOrTargetPitches works here because the curve curves
    // from either 0 to target (fade in) or initial to 0 (fade out), and since
    // the curve fits into 0 - 1 in the y axis, multiplying will scale it from
    // 0 to initial or target
    // just plug it into desmos or something
    for (auto& [id, channel] : FMODAudioEngine::get()->m_channelIDToChannel) {
        float multipliedEased = eased * m_initialOrTargetPitches[id];
        channel->setPitch(multipliedEased);
    }

    // setting m_globalChannel's pitch seems to work but seems to be overridden
    // by every channel individually
}
