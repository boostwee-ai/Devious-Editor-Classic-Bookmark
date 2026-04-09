#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <Geode/Geode.hpp>

namespace editor {

enum class ActionType : uint8_t {
    AddObject = 0,
    RemoveObject = 1,
    TransformObject = 2,
    UpdateSettings = 3
};

#pragma pack(push, 1)
struct ObjectData {
    int uniqueID;
    int objectID;
    float posX;
    float posY;
    float rotation;
    float scaleX;
    float scaleY;
    bool flipX;
    bool flipY;
    int zOrder;
    int zLayer;
    int editorLayer1;
    int editorLayer2;
    // For a robust real implementation we would serialize all property string data or group IDs.
    // For this prototype, saving string encoding is robust.
};
#pragma pack(pop)

class ActionSerializer {
public:
    static std::string serializeObject(GameObject* obj);
    static void deserializeAndApplyObject(const std::string& data, LevelEditorLayer* lel);

    static std::string serializeRemove(int uniqueID);
    static int deserializeRemove(const std::string& data);

    static std::string serializeTransform(GameObject* obj);
    static void deserializeTransform(const std::string& data, LevelEditorLayer* lel);
    
    // Convert current level string
    static std::string serializeLevel(LevelEditorLayer* lel);
    static void deserializeAndApplyLevel(const std::string& data, LevelEditorLayer* lel);
};

} // namespace editor
