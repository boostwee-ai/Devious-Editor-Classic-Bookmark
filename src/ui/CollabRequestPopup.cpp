#include "CollabRequestPopup.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include "../network/Session.hpp"
#include "../network/Protocol.hpp"

using namespace geode::prelude;

namespace ui {

CollabRequestPopup* CollabRequestPopup::create(std::string const& username) {
    auto ret = new CollabRequestPopup();
    if (ret && ret->init(username)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CollabRequestPopup::init(std::string const& username) {
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    auto darken = CCLayerColor::create(ccc4(0, 0, 0, 150));
    this->addChild(darken);

    m_mainLayer = CCLayer::create();
    this->addChild(m_mainLayer);

    auto bg = CCScale9Sprite::create("GJ_square01.png", { 0, 0, 80, 80 });
    bg->setContentSize({ 300.f, 200.f });
    bg->setPosition(winSize / 2);
    m_mainLayer->addChild(bg);

    auto title = CCLabelBMFont::create("Collaboration Request", "goldFont.fnt");
    title->setPosition({winSize.width / 2, winSize.height / 2 + 85.f});
    m_mainLayer->addChild(title);

    auto label = CCLabelBMFont::create(fmt::format("{} would like to collaborate with you.", username).c_str(), "bigFont.fnt");
    label->setScale(0.5f);
    label->setPosition(winSize / 2 + ccp(0, 20));
    m_mainLayer->addChild(label);

    auto yesBtnSprite = ButtonSprite::create("Yes");
    auto noBtnSprite = ButtonSprite::create("No");

    auto yesBtn = CCMenuItemSpriteExtra::create(yesBtnSprite, this, menu_selector(CollabRequestPopup::onYes));
    auto noBtn = CCMenuItemSpriteExtra::create(noBtnSprite, this, menu_selector(CollabRequestPopup::onNo));

    auto menu = CCMenu::create();
    menu->addChild(yesBtn);
    menu->addChild(noBtn);
    menu->alignItemsHorizontallyWithPadding(20.f);
    menu->setPosition(winSize / 2 + ccp(0, -30));
    m_mainLayer->addChild(menu);

    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    return true;
}

void CollabRequestPopup::show() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;
    this->retain();
    scene->addChild(this, 105);
    this->release();
}

void CollabRequestPopup::onYes(cocos2d::CCObject*) {
    network::Session::get().sendPacket(network::Protocol::createPacket(network::PacketType::CollabResponse, "Yes"));
    this->onClose(nullptr);
}

void CollabRequestPopup::onNo(cocos2d::CCObject*) {
    network::Session::get().sendPacket(network::Protocol::createPacket(network::PacketType::CollabResponse, "No"));
    network::Session::get().disconnect();
    this->onClose(nullptr);
}

void CollabRequestPopup::onClose(cocos2d::CCObject*) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void CollabRequestPopup::keyBackClicked() {
    onClose(nullptr);
}

} // namespace ui
