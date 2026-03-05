#include "RewindButton.hpp"
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
    spriteBase->setCascadeOpacityEnabled(true);
    spriteBase->addChildAtPosition(rewindSprite, geode::Anchor::Center, { -3.f, 0.f });

    // copy of addBindSprites from custom keybinds UILayer.cpp but slightly modified
    // keeps it consistent with the other checkpoint buttons
    GLubyte opacity = GameManager::get()->m_practiceOpacity * 255;
    auto bindContainer = CCNode::create();
    bool first = true;
    for (auto& bind : geode::Mod::get()->getSettingValue<std::vector<geode::Keybind>>("keybind")) {
        if (!first) {
            auto separator = cocos2d::CCLabelBMFont::create("/", "bigFont.fnt");
            separator->setScale(.8f);
            separator->setOpacity(opacity);
            bindContainer->addChild(separator);
        }
        first = false;

        auto label = bind.createNode();
        label->setScale(.8f);

        if (auto text = geode::cast::typeinfo_cast<cocos2d::CCLabelBMFont*>(label)) {
            text->setFntFile("bigFont.fnt");
        }

        if (auto rgba = geode::cast::typeinfo_cast<CCRGBAProtocol*>(label)) {
            rgba->setOpacity(opacity);
        }

        bindContainer->addChild(label);
    }

    bindContainer->setID("binds"_spr);
    bindContainer->setContentSize({ spriteBase->getScaledContentWidth(), 40.f });
    bindContainer->setLayout(geode::SimpleRowLayout::create()->setMainAxisScaling(geode::AxisScaling::ScaleDown));
    bindContainer->setAnchorPoint({ .5f, .5f });
    spriteBase->addChildAtPosition(bindContainer, geode::Anchor::Bottom, { 0.f, -1.f });

    bool ret = CCMenuItemSpriteExtra::init(spriteBase, nullptr, nullptr, menu_selector(RewindButton::fakeCallback));
    if (!ret) return false;

    m_scaleMultiplier = 1.1f;

    setCascadeOpacityEnabled(true);

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
