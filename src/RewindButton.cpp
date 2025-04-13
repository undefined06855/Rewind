#include "RewindButton.hpp"
#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif
#include "hooks/GJBaseGameLayer.hpp"

RewindButton* RewindButton::create() {
    auto ret = new RewindButton;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RewindButton::init() {
    setID("rewind-button"_spr);

    auto rewindSprite = cocos2d::CCSprite::createWithSpriteFrameName("edit_leftBtn2_001.png");
    rewindSprite->setScale(1.2f);

    auto spriteBase = cocos2d::CCSprite::create("checkpoint-ui-base.png"_spr);
    spriteBase->addChildAtPosition(rewindSprite, geode::Anchor::Center, { -3.f, 0.f });

#ifndef GEODE_IS_IOS
    // copy of addBindSprites from custom keybinds UILayer.cpp but slightly modified
    // keeps it consistent with the other checkpoint buttons
    auto bindContainer = cocos2d::CCNode::create();
    bindContainer->setScale(.65f);
    bool first = true;
    for (auto& bind : keybinds::BindManager::get()->getBindsFor("rewind"_spr)) {
        if (!first) {
            bindContainer->addChild(cocos2d::CCLabelBMFont::create("/", "bigFont.fnt"));
        }
        first = false;
        bindContainer->addChild(bind->createLabel());
    }
    bindContainer->setID("binds"_spr);
    bindContainer->setContentSize({ spriteBase->getContentSize().width / bindContainer->getScale(), 40.f });
    bindContainer->setLayout(geode::RowLayout::create());
    bindContainer->setAnchorPoint({ .5f, .5f });
    spriteBase->addChildAtPosition(bindContainer, geode::Anchor::Bottom, { 0.f, -1.5f });
#endif

    bool ret = CCMenuItemSpriteExtra::init(spriteBase, nullptr, nullptr, menu_selector(RewindButton::fakeCallback));
    if (!ret) return false;

    m_scaleMultiplier = 1.1f;

    return true;
}

// doesn't matter
void RewindButton::fakeCallback(cocos2d::CCObject* sender) {}

void RewindButton::selected() {
    bool ate = static_cast<HookedGJBaseGameLayer*>(GJBaseGameLayer::get())->rewindStateUpdate(true);
    if (ate) {
        CCMenuItemSpriteExtra::selected();
    }
}

void RewindButton::unselected() {
    static_cast<HookedGJBaseGameLayer*>(GJBaseGameLayer::get())->rewindStateUpdate(false);
    CCMenuItemSpriteExtra::unselected();
}
