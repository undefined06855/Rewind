#pragma once
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <RenderTexture.hpp>

struct RewindFrame {
    geode::Ref<CheckpointObject> m_checkpoint;
    geode::Ref<cocos2d::CCSprite> m_preview;

    // FIXME: player rotation/speed doesnt get set properly from checkpointobject
    // seems to be deeper than just calling setRotation and m_rotationSpeed though
};

class $modify(HookedGJBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<RewindFrame> m_history;
        int m_rewindIndex;

        float m_checkpointTimer;
        float m_rewindTimer;

        bool m_isRewinding;
        bool m_isTransitioningOut;

        float m_secondsPerFrame;
        int m_historyLength;
        float m_resolutionMultiplier;
        bool m_captureInNormalMode;

        geode::Ref<cocos2d::CCLayerGradient> m_bgGradient;
        cocos2d::CCSprite* m_currentPreview;

        Fields();
        ~Fields();
    };

    bool init();
    void update(float dt);

    void addRewindFrame();
    void tickRewind();

    bool rewindStateUpdate(bool down);

    void startRewind();
    void commitRewind();

    void setGameplayLayersVisible(bool visible);
};
