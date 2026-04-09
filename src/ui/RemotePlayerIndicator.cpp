#include "RemotePlayerIndicator.hpp"

using namespace geode::prelude;

namespace ui {

RemotePlayerIndicator* RemotePlayerIndicator::create() {
    auto ret = new RemotePlayerIndicator();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool RemotePlayerIndicator::init() {
    if (!CCNode::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_line = CCLayerColor::create({255, 255, 255, 150}, winSize.width * 10, 2);
    m_line->ignoreAnchorPointForPosition(false);
    m_line->setAnchorPoint({0.5f, 0.5f});
    this->addChild(m_line);

    m_icon = CCSprite::createWithSpriteFrameName("gj_icon_01_001.png"); // basic icon
    if (m_icon) {
        m_icon->setOpacity(150);
        this->addChild(m_icon);
    }

    return true;
}

void RemotePlayerIndicator::updatePosition(float x, float y, float zoom) {
    // The given X and Y are usually in editor world coords, need to match visually
    this->setPosition({x, y});
    this->setScale(1.f / zoom); // counteract zoom if needed so it stays same visual size
}

} // namespace ui
