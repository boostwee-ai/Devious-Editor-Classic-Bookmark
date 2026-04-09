#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

namespace ui {

class CollabRequestPopup : public geode::Popup {
    std::string m_username;
protected:
    bool init(float width, float height, std::string const& username);

    void onYes(cocos2d::CCObject*);
    void onNo(cocos2d::CCObject*);

public:
    static CollabRequestPopup* create(std::string const& username);
};

} // namespace ui
