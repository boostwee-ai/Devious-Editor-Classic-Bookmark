#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

namespace ui {

class PlayerBrowserLayer : public geode::Popup<> {
protected:
    bool setup() override;
    void onClose(cocos2d::CCObject*) override;

    void updateList(float dt);
    
    cocos2d::CCNode* m_listLayer;

public:
    static PlayerBrowserLayer* create();
};

} // namespace ui
