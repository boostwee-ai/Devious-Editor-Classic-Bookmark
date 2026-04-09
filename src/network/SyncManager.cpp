#include "SyncManager.hpp"
#include "Session.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace network {

SyncManager& SyncManager::get() {
    static SyncManager instance;
    return instance;
}

void SyncManager::setupCallbacks() {
    Session::get().setPacketCallback([this](const PacketHeader& header, const std::vector<uint8_t>& payload) {
        handleIncomingPacket(header, payload);
    });
}

void SyncManager::sendAction(editor::ActionType type, const std::string& serializedData) {
    if (m_internalSync) return; // Prevent echoing back received actions
    
    matjson::Value json;
    json["type"] = static_cast<int>(type);
    json["data"] = serializedData;
    
    auto packet = Protocol::createPacket(PacketType::EditorAction, json.dump(matjson::NO_INDENTATION));
    Session::get().sendPacket(packet);
}

void SyncManager::sendViewportPosition(float x, float y, float zoom) {
    matjson::Value json;
    json["v_x"] = x;
    json["v_y"] = y;
    json["zoom"] = zoom;

    auto packet = Protocol::createPacket(PacketType::PlayerPositionUpdate, json.dump(matjson::NO_INDENTATION));
    Session::get().sendPacket(packet);
}

void SyncManager::sendLevelSync() {
    auto lel = LevelEditorLayer::get();
    if (!lel) return;

    std::string levelData = editor::ActionSerializer::serializeLevel(lel);
    auto packet = Protocol::createPacket(PacketType::LevelSync, levelData);
    Session::get().sendPacket(packet);
}

void SyncManager::sendSessionLeave() {
    auto packet = Protocol::createPacket(PacketType::SessionLeave, "");
    Session::get().sendPacket(packet);
}

void SyncManager::handleIncomingPacket(const PacketHeader& header, const std::vector<uint8_t>& payload) {
    std::string payloadStr(payload.begin(), payload.end());

    m_internalSync = true; // Block outgoing echoes while processing

    auto lel = LevelEditorLayer::get();

    switch (header.type) {
        case PacketType::PlayerPositionUpdate: {
            auto jsonRes = matjson::parse(payloadStr);
            if (jsonRes.isOk()) {
                auto json = jsonRes.unwrap();
                m_remoteViewportX = static_cast<float>(json["v_x"].asDouble().unwrapOr(0.0));
                m_remoteViewportY = static_cast<float>(json["v_y"].asDouble().unwrapOr(0.0));
                m_remoteZoom = static_cast<float>(json["zoom"].asDouble().unwrapOr(1.0));
            }
            break;
        }
        case PacketType::EditorAction: {
            if (!lel) break;
            auto jsonRes = matjson::parse(payloadStr);
            if (jsonRes.isOk()) {
                auto json = jsonRes.unwrap();
                auto type = static_cast<editor::ActionType>(json["type"].asInt().unwrapOr(0));
                auto data = json["data"].asString().unwrapOr("");

                if (type == editor::ActionType::AddObject) {
                    editor::ActionSerializer::deserializeAndApplyObject(data, lel);
                } else if (type == editor::ActionType::RemoveObject) {
                    int uid = editor::ActionSerializer::deserializeRemove(data);
                    if (uid != -1) {
                        for (auto item : CCArrayExt<GameObject*>(lel->m_objects)) {
                            if (item->m_uniqueID == uid) {
                                lel->m_editorUI->deleteObject(item, false);
                                break;
                            }
                        }
                    }
                } else if (type == editor::ActionType::TransformObject) {
                    editor::ActionSerializer::deserializeTransform(data, lel);
                }
            }
            break;
        }
        case PacketType::SessionLeave: {
            FLAlertLayer::create("Collab Status", "The host has left the session.", "OK")->show();
            Session::get().disconnect();
            break;
        }
        default:
            break;
    }

    m_internalSync = false;
}

} // namespace network
