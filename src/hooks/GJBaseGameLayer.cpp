#include "GJBaseGameLayer.hpp"
#include "../FadeMusicAction.hpp"
#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

HookedGJBaseGameLayer::Fields::Fields()
    : m_history({})
    , m_rewindIndex(0)

    , m_checkpointTimer(0.f)
    , m_rewindTimer(0.f)

    , m_isRewinding(false)
    , m_isTransitioningOut(false)
    
    , m_rewindsCancelled(0)

    , m_secondsPerFrame(1.f / geode::Mod::get()->getSettingValue<int64_t>("frames-per-second"))
    , m_historyLength(geode::Mod::get()->getSettingValue<int64_t>("history-length"))
    , m_resolutionMultiplier(geode::Mod::get()->getSettingValue<double>("frame-resolution"))

    , m_bgGradient(nullptr)
    , m_currentPreview(nullptr) {}

HookedGJBaseGameLayer::Fields::~Fields() {
    if (m_bgGradient) {
        m_bgGradient->removeFromParent();
    }
}

bool HookedGJBaseGameLayer::init() {
    if (!GJBaseGameLayer::init()) return false;

    auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);
    if (!cast) return true;

    auto fields = m_fields.self();

#ifndef GEODE_IS_IOS
    addEventListener<keybinds::InvokeBindFilter>([this](keybinds::InvokeBindEvent* event) {
        bool ate = rewindStateUpdate(event->isDown());
        return ate ? geode::ListenerResult::Stop : geode::ListenerResult::Propagate;
    }, "rewind"_spr);
#endif

    // background gradient
    fields->m_bgGradient = cocos2d::CCLayerGradient::create({ 0, 101, 253, 255 }, { 0, 46, 115, 255 }, { -.5f, -1.f });
    fields->m_bgGradient->setID("bg-gradient"_spr);
    fields->m_bgGradient->setScale(1.f / .9f);
    addChild(fields->m_bgGradient, -1000);

    return true;
}

void HookedGJBaseGameLayer::update(float dt) {
    auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);
    if (!cast) {
        GJBaseGameLayer::update(dt);
        return;
    }

    auto fields = m_fields.self();
    float trueDt = dt / m_gameState.m_timeWarp; // TODO: make this fix the ccscaleto as well?

    if (fields->m_isRewinding) {
        fields->m_rewindTimer += trueDt;

        // change time between ticks to speed up rewind when held for longer
        float rewindSpeed = fields->m_secondsPerFrame;
        if (fields->m_rewindIndex > 10) rewindSpeed /= 2;
        if (fields->m_rewindIndex > 15) rewindSpeed /= 2;

        // tick rewind if enough time passed
        if (fields->m_rewindTimer > rewindSpeed) {
            fields->m_rewindTimer = 0.f;
            tickRewind();
        }
    } else if (!fields->m_isTransitioningOut) {
        GJBaseGameLayer::update(dt);

        fields->m_checkpointTimer += trueDt;

        // tick checkpoint if enough time passed
        if (fields->m_checkpointTimer > fields->m_secondsPerFrame) {
            fields->m_checkpointTimer = 0.f;
            addRewindFrame();
        }
    }
}

bool HookedGJBaseGameLayer::rewindStateUpdate(bool down) {
    auto fields = m_fields.self();
    auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);

    // if paused or in normal mode or transitioning or rewinding (but only on keydown) or level is completing
    if (
        cast->m_isPaused 
        || !cast->m_isPracticeMode
        || fields->m_isTransitioningOut
        || ( down && fields->m_isRewinding )
        || m_levelEndAnimationStarted
    ) {
        fields->m_rewindsCancelled++;
        return false;
    }

    // see comment on m_rewindsCancelled member for more info
    if (fields->m_rewindsCancelled > 0) {
        fields->m_rewindsCancelled--;
        return false;
    }
    
    if (down) {
        fields->m_rewindTimer = 0.f;
        startRewind();
    } else {
        // released key, commit
        commitRewind();
    }

    return true;
}

void HookedGJBaseGameLayer::addRewindFrame() {
    auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);
    auto fields = m_fields.self();
    auto res = cocos2d::CCDirector::get()->getWinSizeInPixels();

    auto rentex = RenderTexture(
        res.width * fields->m_resolutionMultiplier,
        res.height * fields->m_resolutionMultiplier
    ).intoManagedSprite();

    // repeating background breaks in rendertexture, reset pos and capture
    auto origBGPos = m_background->getPosition();
    m_background->setPosition({ 0.f, 0.f });
    rentex->render.capture(m_objectParent, true);
    rentex->render.capture(m_shaderLayer, false);
    rentex->render.capture(m_aboveShaderParent, false);
    rentex->render.capture(m_uiTriggerUI, false);
    m_background->setPosition(origBGPos);

    // set stuff on the sprite
    rentex->sprite->setFlipY(true);
    rentex->sprite->setID("overlay"_spr);
    rentex->sprite->setPosition(getContentSize() / 2.f);
    rentex->sprite->setZOrder(1);
    rentex->sprite->setScale(1.f / fields->m_resolutionMultiplier);

    // and make a frame and push to the front of the deque
    // allows most recent to be index 0, second to be index 1 etc for easy
    // indexing
    float currentSongPitch;
    auto frame = RewindFrame{
        .m_checkpoint = geode::Ref(cast->createCheckpoint()),
        .m_preview = rentex->sprite
    };

    fields->m_history.push_front(frame);

    // shouldnt run more than once really
    while (fields->m_history.size() > fields->m_historyLength) {
        fields->m_history.pop_back();
    }
}

void HookedGJBaseGameLayer::tickRewind() {
    auto fields = m_fields.self();

    // increment and see if we've reached the end
    fields->m_rewindIndex++;
    if (fields->m_rewindIndex >= fields->m_history.size()) {
        fields->m_rewindIndex = fields->m_history.size() - 1;
    }

    // should always be true except for the first preview
    if (fields->m_currentPreview) {
        fields->m_currentPreview->removeFromParent();
    }

    // and add
    auto frame = fields->m_history[fields->m_rewindIndex];
    addChild(frame.m_preview);
    fields->m_currentPreview = frame.m_preview;
}

void HookedGJBaseGameLayer::startRewind() {
    // fix animations getting slown down - correct timewarp gets restored anyway
    applyTimeWarp(1.f);

    runAction(cocos2d::CCScaleTo::create(.2f, .93f));
    setGameplayLayersVisible(false);

    tickRewind(); // add first overlay image

    m_fields->m_isRewinding = true;

    // target of fademusicaction is always assumed to be fmodaudioengine so this works
    // TODO: is dividing by timewarp really necessary for this
    cocos2d::CCScene::get()->runAction(FadeMusicAction::create(.65f, FadeMusicDirection::FadeOut));
}

void HookedGJBaseGameLayer::commitRewind() {
    auto fields = m_fields.self();

    fields->m_isRewinding = false;
    fields->m_isTransitioningOut = true;

    runAction(
        cocos2d::CCSequence::createWithTwoActions(
            cocos2d::CCScaleTo::create(.2f, 1.f),
            geode::cocos::CallFuncExt::create([this, fields]{
                setGameplayLayersVisible(true);

                auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);
                
                // should always be valid at this point but check just in case
                if (fields->m_currentPreview) {
                    fields->m_currentPreview->removeFromParent();
                }
            
                // resetlevel loads the last checkpoint in checkpointArray in
                // practice mode
                auto frame = fields->m_history[fields->m_rewindIndex];
                cast->m_checkpointArray->addObject(frame.m_checkpoint);
                cast->resetLevel();
                cast->m_checkpointArray->removeLastObject();
                m_attempts--; // resetLevel increments attempt counter

                // pop all frames between now and reset pos
                // TODO: make this more efficient? dont want thirty off by one
                // errors
                for (int i = 0; i < fields->m_rewindIndex; i++) fields->m_history.pop_front();
                fields->m_rewindIndex = 0;

                // and reset members
                fields->m_isTransitioningOut = false;
                fields->m_checkpointTimer = 0.f;
                fields->m_currentPreview = nullptr;

                // fade music back in to pitches stored in m_audioState
                // interestingly m_pitches exists but isnt correct? idfk man
                auto states = frame.m_checkpoint->m_audioState.m_soundStateForChannels;
                std::unordered_map<int, float> pitches = {};
                for (auto& [channel, state] : states) { pitches[channel] = state.m_speed; }
                cocos2d::CCScene::get()->runAction(FadeMusicAction::create(.65f, FadeMusicDirection::FadeIn, pitches));
            })
        )
    );
}

void HookedGJBaseGameLayer::setGameplayLayersVisible(bool visible) {
    m_objectParent->setVisible(visible);
    m_shaderLayer->setVisible(visible);
    m_aboveShaderParent->setVisible(visible);
    m_uiTriggerUI->setVisible(visible);
}
