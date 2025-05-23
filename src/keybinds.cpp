#ifndef GEODE_IS_IOS
#include <geode.custom-keybinds/include/Keybinds.hpp>

$on_mod(Loaded) {
	keybinds::BindManager::get()->registerBindable({
		"rewind"_spr,
		"Rewind",
		"",
		{ keybinds::Keybind::create(cocos2d::KEY_C), keybinds::ControllerBind::create(cocos2d::CONTROLLER_Y) },
		keybinds::Category::PLAY,
		false
	});
}

#endif
