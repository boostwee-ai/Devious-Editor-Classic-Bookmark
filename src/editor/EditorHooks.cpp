#include "EditorHooks.hpp"
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include "../ui/CollabButton.hpp"
#include "../ui/RemotePlayerIndicator.hpp"
#include "../network/SyncManager.hpp"
#include "../network/Session.hpp"
#include "../network/Discovery.hpp"

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = CCMenu::create();
        auto btnSprite = ButtonSprite::create("Collab");
        auto btn = CCMenuItemSpriteExtra::create(btnSprite, this, menu_selector(MyEditorUI::onCollabButton));
        menu->addChild(btn);
        
        menu->setPosition({ CCDirector::sharedDirector()->getWinSize().width - 40.f, 40.f });
        this->addChild(menu);

        return true;
    }

    void onCollabButton(cocos2d::CCObject* sender) {
        ui::CollabButton::onCollabButton(sender);
    }

    void moveObject(GameObject* p0, cocos2d::CCPoint p1) {
        EditorUI::moveObject(p0, p1);
        if (p0 && !network::SyncManager::get().isSyncing()) {
            network::SyncManager::get().sendAction(editor::ActionType::TransformObject, editor::ActionSerializer::serializeTransform(p0));
        }
    }

    void scaleObjects(cocos2d::CCArray* p0, float p1, cocos2d::CCPoint p2, bool p3, bool p4, bool p5) {
        EditorUI::scaleObjects(p0, p1, p2, p3, p4, p5);
        if (!network::SyncManager::get().isSyncing()) {
            for (auto item : CCArrayExt<GameObject*>(p0)) {
                network::SyncManager::get().sendAction(editor::ActionType::TransformObject, editor::ActionSerializer::serializeTransform(item));
            }
        }
    }
};

class $modify(MyLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        ui::RemotePlayerIndicator* m_indicator;
    };

    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelEditorLayer::init(p0, p1)) return false;

        m_fields->m_indicator = ui::RemotePlayerIndicator::create();
        this->m_objectLayer->addChild(m_fields->m_indicator);
        m_fields->m_indicator->setZOrder(9999);
        
        this->schedule(schedule_selector(MyLevelEditorLayer::updateSync), 0.1f);

        network::Session::get().startHost();
        network::SyncManager::get().setupCallbacks();
        
        bool collabEnabled = Mod::get()->getSettingValue<bool>("enable_collab");
        std::string user = GJAccountManager::sharedState()->m_username;
        if (user.empty()) user = "Player";
        network::Discovery::get().setLocalState(user, true, collabEnabled);

        return true;
    }

    void updateSync(float dt) {
        auto objLayer = this->m_objectLayer;
        float x = -objLayer->getPositionX() / objLayer->getScale();
        float y = -objLayer->getPositionY() / objLayer->getScale();
        
        network::SyncManager::get().sendViewportPosition(x, y, objLayer->getScale());

        if (m_fields->m_indicator) {
            m_fields->m_indicator->updatePosition(
                network::SyncManager::get().m_remoteViewportX,
                network::SyncManager::get().m_remoteViewportY,
                network::SyncManager::get().m_remoteZoom
            );
        }
    }

    void addSpecial(GameObject* p0) {
        LevelEditorLayer::addSpecial(p0);
        if (p0 && !network::SyncManager::get().isSyncing()) {
            network::SyncManager::get().sendAction(editor::ActionType::AddObject, editor::ActionSerializer::serializeObject(p0));
        }
    }

    void removeObject(GameObject* p0, bool p1) {
        LevelEditorLayer::removeObject(p0, p1);
        if (p0 && !network::SyncManager::get().isSyncing()) {
            network::SyncManager::get().sendAction(editor::ActionType::RemoveObject, editor::ActionSerializer::serializeRemove(p0->m_uniqueID));
        }
    }
};

class $modify(MyEditorPauseLayer, EditorPauseLayer) {
    void onExitEditor(cocos2d::CCObject* sender) {
        network::SyncManager::get().sendSessionLeave();
        network::Session::get().disconnect();
        
        std::string user = GJAccountManager::sharedState()->m_username;
        if (user.empty()) user = "Player";
        
        bool collabEnabled = Mod::get()->getSettingValue<bool>("enable_collab");
        network::Discovery::get().setLocalState(user, false, collabEnabled);
        
        EditorPauseLayer::onExitEditor(sender);
    }
};
