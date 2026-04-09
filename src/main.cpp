#include <Geode/Geode.hpp>
#include "network/Discovery.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    if (Mod::get()->getSettingValue<bool>("enable_collab")) {
        network::Discovery::get().start();
    }
}
