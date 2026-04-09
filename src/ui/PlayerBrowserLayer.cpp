#include "PlayerBrowserLayer.hpp"
#include "../network/Discovery.hpp"
#include "../network/Session.hpp"
#include "../network/Protocol.hpp"

using namespace geode::prelude;

namespace ui {

PlayerBrowserLayer* PlayerBrowserLayer::create() {
    auto ret = new PlayerBrowserLayer();
    if (ret && ret->initAnchored(360.f, 240.f, "GJ_square01.png")) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PlayerBrowserLayer::setup() {
    m_noElasticity = true;
    this->setTitle("LAN Players");
    
    m_listLayer = CCNode::create();
    m_listLayer->setContentSize({340.f, 180.f});
    m_listLayer->setPosition({
        this->m_mainLayer->getContentSize().width / 2,
        this->m_mainLayer->getContentSize().height / 2 - 10
    });
    m_listLayer->setAnchorPoint({0.5f, 0.5f});
    this->m_mainLayer->addChild(m_listLayer);

    this->schedule(schedule_selector(PlayerBrowserLayer::updateList), 1.0f);
    updateList(0.f);

    return true;
}

void PlayerBrowserLayer::onClose(cocos2d::CCObject* sender) {
    this->geode::Popup<>::onClose(sender);
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
            // Initiate connection and send request
            if (network::Session::get().connectToPeer(peer.ip)) {
                matjson::Value json;
                json["user"] = network::Discovery::get().getActivePeers().empty() ? "Me" : "Host";
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
