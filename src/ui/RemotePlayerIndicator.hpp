#pragma once

#include <Geode/Geode.hpp>

namespace ui {

class RemotePlayerIndicator : public cocos2d::CCNode {
protected:
    cocos2d::CCLayerColor* m_line;
    cocos2d::CCSprite* m_icon;
    
    bool init() override;

public:
    static RemotePlayerIndicator* create();
    void updatePosition(float x, float y, float zoom);
};

} // namespace ui
