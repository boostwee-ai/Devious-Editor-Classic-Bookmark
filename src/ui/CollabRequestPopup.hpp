#pragma once

#include <Geode/Geode.hpp>

namespace ui {

class CollabRequestPopup : public geode::Popup<std::string const&> {
protected:
    bool setup(std::string const& username) override;

    void onYes(cocos2d::CCObject*);
    void onNo(cocos2d::CCObject*);

public:
    static CollabRequestPopup* create(std::string const& username);
};

} // namespace ui
