#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

namespace ui {

class CollabRequestPopup : public geode::Popup<std::string const&> {
    using geode::Popup<std::string const&>::m_mainLayer;
    using geode::Popup<std::string const&>::m_noElasticity;
protected:
    bool setup(std::string const& username) override;

    void onYes(cocos2d::CCObject*);
    void onNo(cocos2d::CCObject*);

public:
    static CollabRequestPopup* create(std::string const& username);
};

} // namespace ui
