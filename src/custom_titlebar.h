#pragma once

#include "core/math/color.h"
#include "core/object/object.h"
#include "core/variant/array.h"

/// 自定义标题栏：让窗口保用系统原生边框（拖动、缩放、贴边分屏、圆角、阴影全部由系统提供），
/// 但把标题栏区域并入客户区，由 GDScript 在客户区顶部自绘。
///
/// **不动窗口样式，只改客户区计算**：接管 `WM_NCCALCSIZE` 把客户区扩到整个窗口矩形，
/// 系统就不再绘制标题栏，而 `WS_THICKFRAME` 仍归系统所有——因此缩放边框、Aero Snap、
/// 双击最大化、最小化/最大化动画全部保持原生行为，不经 GDScript 逐帧算坐标。
/// **样式必须保持原样**：引擎的 `_update_window_style()` 会在切换窗口模式时按
/// `WS_OVERLAPPEDWINDOW` 整块重写 `GWL_STYLE`，改过的样式会被覆盖回去。
///
/// 仅在 Windows 上可用。
class CustomTitlebar : public Object {
	GDCLASS(CustomTitlebar, Object);

	/// 进程内唯一实例。
	static CustomTitlebar *singleton;

	/// 标题栏高度，客户区坐标下的像素值。
	float caption_height = 0.0f;

	/// 顶边缩放带厚度：命中测试里 y 小于它时报 [code]HTTOP[/code] 供拖拽改高。
	/// **必须远小于标题栏高度**：若与标题栏等高，按钮所在行会先被判成缩放带，
	/// 按钮就永远点不动。
	float border_thickness = 4.0f;

	/// 标题栏内仍需接收鼠标的区域（按钮等），客户区坐标。
	Array interactive_rects;

	/// 原生窗口句柄，以 void* 保存以避免头文件引入 windows.h。
	void *window_handle = nullptr;

	/// 被替换前的窗口过程，未安装时为 nullptr。
	void *original_proc = nullptr;

	/// 判断给定消息是否在拦截范围内。
	bool _is_installed() const;

protected:
	static void _bind_methods();

public:
	static CustomTitlebar *get_singleton() { return singleton; }

	CustomTitlebar();
	~CustomTitlebar();

	/// 当前平台是否支持自定义标题栏。
	bool is_supported() const;

	/// 安装：替换窗口过程并把标题栏并入客户区。
	/// 返回 false 表示系统不支持或取不到窗口句柄。
	bool install();

	/// 卸载：恢复原生标题栏与原始窗口过程。
	void uninstall();

	/// 当前是否已安装。
	bool is_installed() const { return _is_installed(); }

	/// 隐藏窗口（只剩托盘图标）。
	/// **这是真正的隐藏**：`ShowWindow(SW_HIDE)` 会让窗口同时退出任务栏与 Alt-Tab，
	/// 而不是「最小化」那种还留在任务栏里的假隐藏。
	/// 未安装或取不到句柄时返回 false。
	bool hide_window();

	/// 显示并激活窗口（从托盘恢复）。
	/// **必须先 `SW_SHOW` 再抢前台**：隐藏态的窗口直接抢前台是无效的。
	bool show_window();

	/// 处理一条窗口消息。**仅供窗口过程调用**。
	/// 返回 true 表示已处理，调用方应直接采用 [param r_result]；false 表示交回原窗口过程。
	bool _handle_window_message(int64_t p_message, int64_t p_wparam, int64_t p_lparam, int64_t &r_result);

	/// 取被替换前的窗口过程。**仅供窗口过程调用**。
	void *_get_original_proc() const { return original_proc; }

	/// 设置标题栏高度。
	/// [param height] 高度，客户区像素。
	void set_caption_height(float p_height);

	/// 取标题栏高度。
	float get_caption_height() const { return caption_height; }

	/// 设置顶部缩放带厚度。
	/// **必须明显小于标题栏高度**：命中测试里 y 小于它的位置报成可拖拽改高，
	/// 与标题栏等高时按钮会被判成缩放带而点不动。
	/// [param thickness] 厚度，客户区像素。
	void set_border_thickness(float p_thickness);

	/// 取顶部缩放带厚度。
	float get_border_thickness() const { return border_thickness; }

	/// 设置标题栏内需要接收鼠标的区域（按钮等）。
	/// 落在这些矩形内的位置不会报成标题栏，点击才会交给界面控件。
	/// [param rects] 矩形数组，客户区坐标。
	void set_interactive_rects(const Array &p_rects);

	/// 取标题栏内的交互区域。
	Array get_interactive_rects() const { return interactive_rects; }

	/// 设置窗口边框色。
	/// [param color] 边框颜色；alpha 为 0 表示沿用系统默认。
	void set_border_color(const Color &p_color);

	/// 设置窗口圆角。
	/// [param rounded] 为 true 显式指定圆角（DWMWCP_ROUND）。**不要用 DEFAULT**：
	/// 标题栏并入客户区后系统会判定无标准边框可圆，表现为启动后圆角消失。
	void set_rounded_corners(bool p_rounded);

	/// 设置窗口深浅色外观，影响系统绘制的边框、阴影与右键菜单配色。
	/// [param dark] 为 true 使用深色外观。
	void set_dark_appearance(bool p_dark);

	/// 窗口当前是否处于最大化。
	bool is_maximized() const;
};
