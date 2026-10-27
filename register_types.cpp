#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "custom_titlebar.h"

// 模块初始化入口，由引擎构建系统在相应阶段调用。
// 单例与类都在 SCENE 阶段注册：此时 DisplayServer 已就绪，可安全取窗口句柄。
void initialize_custom_titlebar_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<CustomTitlebar>();
	CustomTitlebar *titlebar = memnew(CustomTitlebar);
	Engine::get_singleton()->add_singleton(Engine::Singleton("CustomTitlebar", titlebar));
}

// 模块卸载入口：先摘掉单例再释放，避免 GDScript 侧拿到悬空引用。
void uninitialize_custom_titlebar_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	CustomTitlebar *titlebar = CustomTitlebar::get_singleton();
	if (titlebar == nullptr) {
		return;
	}

	// **先卸载再释放**：卸载会把窗口过程还原，否则窗口消息仍打到即将释放的对象上。
	titlebar->uninstall();
	Engine::get_singleton()->remove_singleton("CustomTitlebar");
	memdelete(titlebar);
}
