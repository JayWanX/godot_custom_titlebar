#ifndef CUSTOM_TITLEBAR_REGISTER_TYPES_H
#define CUSTOM_TITLEBAR_REGISTER_TYPES_H

#include "modules/register_module_types.h"

// 模块初始化入口：在 SCENE 阶段注册本模块的类与单例。
void initialize_custom_titlebar_module(ModuleInitializationLevel p_level);
// 模块卸载入口：注销单例并释放实例。
void uninitialize_custom_titlebar_module(ModuleInitializationLevel p_level);

#endif // CUSTOM_TITLEBAR_REGISTER_TYPES_H
