#include "CollabButton.hpp"
#include "PlayerBrowserLayer.hpp"
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

namespace ui {

void CollabButton::onCollabButton(cocos2d::CCObject* sender) {
    auto p = ui::PlayerBrowserLayer::create();
    if (p) {
        static_cast<FLAlertLayer*>(p)->show();
    }
}

} // namespace ui
