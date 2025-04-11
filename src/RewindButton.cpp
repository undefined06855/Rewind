#include "RewindButton.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include "GJBaseGameLayer.hpp"

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

    auto rewindLabel = cocos2d::CCLabelBMFont::create("Rewind", "bigFont.fnt");
    rewindLabel->setScale(.2f);

    auto rewindSprite = cocos2d::CCSprite::createWithSpriteFrameName("edit_ccwBtn_001.png");
    rewindSprite->addChildAtPosition(rewindLabel, geode::Anchor::Center);

    auto spriteBase = cocos2d::CCSprite::create("checkpoint-ui-base.png"_spr);

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

    spriteBase->addChildAtPosition(rewindSprite, geode::Anchor::Center);
    
    return CCMenuItemSpriteExtra::init(spriteBase, nullptr, nullptr, menu_selector(RewindButton::fakeCallback));
}

// never gets called anyway
void RewindButton::fakeCallback(cocos2d::CCObject*) {}

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
