#pragma once

#include <Geode/Geode.hpp>

namespace ui {

class CollabRequestPopup : public cocos2d::CCLayer {
protected:
    bool init(std::string const& username);
    void onYes(cocos2d::CCObject*);
    void onNo(cocos2d::CCObject*);
    void onClose(cocos2d::CCObject*);

    cocos2d::CCLayer* m_mainLayer;

public:
    static CollabRequestPopup* create(std::string const& username);
    void show();
    void keyBackClicked() override;
};

} // namespace ui
