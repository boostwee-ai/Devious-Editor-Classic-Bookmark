#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

namespace ui {

class PlayerBrowserLayer : public geode::Popup {
protected:
    bool init(float width, float height);
    
    void updateList(float dt);
    
    cocos2d::CCNode* m_listLayer;

public:
    static PlayerBrowserLayer* create();
};

} // namespace ui
