#include "CollabRequestPopup.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include "../network/Session.hpp"
#include "../network/Protocol.hpp"

using namespace geode::prelude;

namespace ui {

CollabRequestPopup* CollabRequestPopup::create(std::string const& username) {
    auto ret = new CollabRequestPopup();
    if (ret && ret->initAnchored(300.f, 200.f, username)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CollabRequestPopup::setup(std::string const& username) {
    m_noElasticity = true;
    this->setTitle("Collaboration Request");

    auto label = CCLabelBMFont::create(fmt::format("{} would like to collaborate with you.", username).c_str(), "bigFont.fnt");
    label->setScale(0.5f);
    label->setPosition(
        this->m_mainLayer->getContentSize().width / 2,
        this->m_mainLayer->getContentSize().height / 2 + 20
    );
    this->m_mainLayer->addChild(label);

    auto yesBtnSprite = ButtonSprite::create("Yes");
    auto noBtnSprite = ButtonSprite::create("No");

    auto yesBtn = CCMenuItemSpriteExtra::create(yesBtnSprite, this, menu_selector(CollabRequestPopup::onYes));
    auto noBtn = CCMenuItemSpriteExtra::create(noBtnSprite, this, menu_selector(CollabRequestPopup::onNo));

    auto menu = CCMenu::create();
    menu->addChild(yesBtn);
    menu->addChild(noBtn);
    menu->alignItemsHorizontallyWithPadding(20.f);
    menu->setPosition({
        this->m_mainLayer->getContentSize().width / 2,
        this->m_mainLayer->getContentSize().height / 2 - 30
    });
    this->m_mainLayer->addChild(menu);

    return true;
}

void CollabRequestPopup::onYes(cocos2d::CCObject*) {
    // Send response and setup sync
    network::Session::get().sendPacket(network::Protocol::createPacket(network::PacketType::CollabResponse, "Yes"));
    this->onClose(nullptr);
}

void CollabRequestPopup::onNo(cocos2d::CCObject*) {
    // Decline
    network::Session::get().sendPacket(network::Protocol::createPacket(network::PacketType::CollabResponse, "No"));
    network::Session::get().disconnect();
    this->onClose(nullptr);
}

} // namespace ui
