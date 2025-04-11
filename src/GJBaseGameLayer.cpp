#include "GJBaseGameLayer.hpp"
#include "FadeMusicAction.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>

HookedGJBaseGameLayer::Fields::Fields()
    : m_history({})
    , m_rewindIndex(0)

    , m_checkpointTimer(0.f)
    , m_rewindTimer(0.f)

    , m_isRewinding(false)
    , m_isTransitioningOut(false)
    
    , m_rewindsCancelled(0)
    
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
    /* "rewind"_spr */ 
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

    if (fields->m_isRewinding) {
        // rewind timer
        fields->m_rewindTimer += dt;

        // change time between ticks to speed up rewind when held for longer
        float rewindSpeed = .05f;
        if (fields->m_rewindIndex > 10) rewindSpeed = .02f;
        if (fields->m_rewindIndex > 15) rewindSpeed = .01f;

        // tick rewind if enough time passed
        if (fields->m_rewindTimer > rewindSpeed) {
            fields->m_rewindTimer = 0.f;
            tickRewind();
        }
    } else if (!fields->m_isTransitioningOut) {
        GJBaseGameLayer::update(dt);

        // checkpoint timer
        fields->m_checkpointTimer += dt;

        // tick checkpoint if enough time passed
        if (fields->m_checkpointTimer > .05f) {
            fields->m_checkpointTimer = 0.f;
            addRewindFrame();
        }
    }
}

bool HookedGJBaseGameLayer::rewindStateUpdate(bool down) {
    auto fields = m_fields.self();
    auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);

    // if paused or in normal mode or transitioning or rewinding (but only on keydown)
    if (
        cast->m_isPaused 
        || !cast->m_isPracticeMode
        || fields->m_isTransitioningOut
        || ( down && fields->m_isRewinding )
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

    // GL_RGB used here since we dont need alpha and we have a lot of these
    // not sure how much memory/performance this really saves though but it's
    // free so might as well
    auto rentex = RenderTexture(res.width, res.height, 0x80E0, 0x80E0).intoManagedSprite();

    // repeating background breaks in render texture, reset pos and capture
    auto origBGPos = m_background->getPosition();
    m_background->setPosition({ 0.f, 0.f });
    rentex->render.capture(getChildByID("main-node"));
    m_background->setPosition(origBGPos);

    // set stuff on the sprite
    rentex->sprite->setFlipY(true);
    rentex->sprite->setID("overlay"_spr);
    rentex->sprite->setPosition(getContentSize() / 2.f);
    rentex->sprite->setZOrder(1);

    // and make a frame and push back
    float currentSongPitch;
    auto frame = RewindFrame{
        .m_checkpoint = geode::Ref(cast->createCheckpoint()),
        .m_preview = rentex->sprite,

        .m_player1Rotation = m_player1->getRotation(),
        .m_player2Rotation = m_player2->getRotation(),
    };

    fields->m_history.push_front(frame);

    // shouldnt run more than once really
    while (fields->m_history.size() > 150) {
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
    if (auto preview = fields->m_currentPreview.lock()) {
        preview->removeFromParent();
    }

    // and add
    auto frame = fields->m_history[fields->m_rewindIndex];
    addChild(frame.m_preview);
    fields->m_currentPreview = frame.m_preview;
}

void HookedGJBaseGameLayer::startRewind() {
    runAction(cocos2d::CCScaleTo::create(.2f, .9f));
    m_objectLayer->getParent()->setVisible(false);

    tickRewind(); // add first overlay image

    m_fields->m_isRewinding = true;

    // cant just use gjbgl actionmanager since itll get paused when rewinding
    // so using ccscene (or whatever gjbgl parent is)
    getParent()->getActionManager()->addAction(FadeMusicAction::create(.8f, FadeMusicDirection::FadeOut), FMODAudioEngine::get(), false);
}

void HookedGJBaseGameLayer::commitRewind() {
    auto fields = m_fields.self();

    fields->m_isRewinding = false;
    fields->m_isTransitioningOut = true;

    runAction(
        cocos2d::CCSequence::create(
            cocos2d::CCScaleTo::create(.2f, 1.f),
            // cocos2d::CCDelayTime::create(.1f), // unsure on whether to keep this
            geode::cocos::CallFuncExt::create([this, fields]{
                m_objectLayer->getParent()->setVisible(true);

                auto cast = geode::cast::typeinfo_cast<PlayLayer*>(this);
                
                // should always be valid at this point but check just in case
                if (auto preview = fields->m_currentPreview.lock()) {
                    preview->removeFromParent();
                }
            
                // resetlevel loads the last checkpoint in checkpointArray in
                // practice mode
                auto frame = fields->m_history[fields->m_rewindIndex];
                cast->m_checkpointArray->addObject(frame.m_checkpoint);
                cast->resetLevel();
                cast->m_checkpointArray->removeLastObject();
                m_player1->setRotation(frame.m_player1Rotation);
                m_player2->setRotation(frame.m_player2Rotation);
                m_attempts--;
            
                // pop all frames between now and reset pos
                for (int i = 0; i < fields->m_rewindIndex; i++) fields->m_history.pop_front();
                fields->m_rewindIndex = 0;

                // and reset members
                fields->m_isTransitioningOut = false;
                fields->m_checkpointTimer = 0.f;

                getParent()->getActionManager()->addAction(FadeMusicAction::create(.8f, FadeMusicDirection::FadeIn), FMODAudioEngine::get(), false);
            }),
            nullptr
        )
    );
}
