#pragma once

#include <Geode/Geode.hpp>

namespace ui {

class PlayerBrowserLayer : public cocos2d::CCLayer {
protected:
    bool init() override;
    void onClose(cocos2d::CCObject* sender);
    
    void updateList(float dt);
    
    cocos2d::CCNode* m_listLayer;
    cocos2d::CCLayer* m_mainLayer;

public:
    static PlayerBrowserLayer* create();
    void show();
    
    void keyBackClicked() override;
};

} // namespace ui
