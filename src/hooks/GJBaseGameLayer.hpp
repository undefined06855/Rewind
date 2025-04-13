#pragma once
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <RenderTexture.hpp>

struct RewindFrame {
    geode::Ref<CheckpointObject> m_checkpoint;
    geode::Ref<cocos2d::CCSprite> m_preview;

    // dont get properly set from CheckpointObject
    float m_player1Rotation;
    float m_player2Rotation;
};

class $modify(HookedGJBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<RewindFrame> m_history;
        int m_rewindIndex;

        float m_checkpointTimer;
        float m_rewindTimer;
        
        bool m_isRewinding;
        bool m_isTransitioningOut;

        // imagine you start, then start again (should get cancelled), then
        // start again (should get cancelled) - this is why this is an int, so
        // it can count the amount of cancelled rewinds
        int m_rewindsCancelled;

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
