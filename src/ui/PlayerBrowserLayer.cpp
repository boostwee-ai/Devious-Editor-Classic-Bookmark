#include "PlayerBrowserLayer.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include "../network/Discovery.hpp"
#include "../network/Session.hpp"
#include "../network/Protocol.hpp"

using namespace geode::prelude;

namespace ui {

PlayerBrowserLayer* PlayerBrowserLayer::create() {
    auto ret = new PlayerBrowserLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PlayerBrowserLayer::init() {
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Darken background
    auto darken = CCLayerColor::create(ccc4(0, 0, 0, 150));
    this->addChild(darken);

    m_mainLayer = CCLayer::create();
    this->addChild(m_mainLayer);

    // Background sprite
    auto bg = CCScale9Sprite::create("GJ_square01.png", { 0, 0, 80, 80 });
    bg->setContentSize({ 360.f, 240.f });
    bg->setPosition(winSize / 2);
    m_mainLayer->addChild(bg);

    // Title
    auto title = CCLabelBMFont::create("LAN Players", "goldFont.fnt");
    title->setPosition({winSize.width / 2, winSize.height / 2 + 105.f});
    m_mainLayer->addChild(title);

    // Close button
    auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    auto closeBtn = CCMenuItemSpriteExtra::create(
        closeSprite,
        this,
        menu_selector(PlayerBrowserLayer::onClose)
    );
    auto closeMenu = CCMenu::create();
    closeMenu->addChild(closeBtn);
    closeMenu->setPosition({winSize.width / 2 - 170.f, winSize.height / 2 + 110.f});
    m_mainLayer->addChild(closeMenu);
    
    m_listLayer = CCNode::create();
    m_listLayer->setContentSize({340.f, 180.f});
    m_listLayer->setPosition(winSize / 2 + ccp(0, -10));
    m_listLayer->setAnchorPoint({0.5f, 0.5f});
    m_mainLayer->addChild(m_listLayer);

    this->schedule(schedule_selector(PlayerBrowserLayer::updateList), 1.0f);
    updateList(0.f);

    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    return true;
}

void PlayerBrowserLayer::show() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;
    this->retain();
    scene->addChild(this, 100);
    this->release();
}

void PlayerBrowserLayer::onClose(cocos2d::CCObject* sender) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void PlayerBrowserLayer::keyBackClicked() {
    onClose(nullptr);
}

void PlayerBrowserLayer::updateList(float dt) {
    m_listLayer->removeAllChildrenWithCleanup(true);
    
    auto peers = network::Discovery::get().getActivePeers();
    if (peers.empty()) {
        auto noPeersLabel = CCLabelBMFont::create("No players found on LAN.", "bigFont.fnt");
        noPeersLabel->setScale(0.5f);
        noPeersLabel->setPosition(m_listLayer->getContentSize() / 2);
        m_listLayer->addChild(noPeersLabel);
        return;
    }

    float y = m_listLayer->getContentSize().height - 20.f;
    for (const auto& peer : peers) {
        auto label = CCLabelBMFont::create(fmt::format("{} ({})", peer.username, peer.ip).c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition({10.f, y});
        m_listLayer->addChild(label);
        
        auto btnSprite = ButtonSprite::create("Collab");
        btnSprite->setScale(0.5f);
        
        auto btn = CCMenuItemExt::createSpriteExtra(btnSprite, [this, peer](auto) {
            if (network::Session::get().connectToPeer(peer.ip)) {
                auto username = GJAccountManager::sharedState()->m_username;
                if (username.empty()) username = "Player";
                
                matjson::Value json;
                json["user"] = username;
                std::string payload = json.dump(matjson::NO_INDENTATION);
                auto packet = network::Protocol::createPacket(network::PacketType::CollabRequest, payload);
                network::Session::get().sendPacket(packet);
                FLAlertLayer::create("Request Sent", "Waiting for response...", "OK")->show();
            } else {
                FLAlertLayer::create("Error", "Could not connect to peer.", "OK")->show();
            }
        });

        auto menu = CCMenu::create();
        menu->setPosition({300.f, y});
        menu->addChild(btn);
        m_listLayer->addChild(menu);

        y -= 30.f;
    }
}

} // namespace ui
