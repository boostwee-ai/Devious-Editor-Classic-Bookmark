#include "CollabButton.hpp"
#include "PlayerBrowserLayer.hpp"

using namespace geode::prelude;

namespace ui {

void CollabButton::onCollabButton(cocos2d::CCObject* sender) {
    ui::PlayerBrowserLayer::create()->show();
}

} // namespace ui
