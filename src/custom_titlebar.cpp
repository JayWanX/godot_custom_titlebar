#include "custom_titlebar.h"

#include "core/object/class_db.h"

#ifdef WINDOWS_ENABLED

#include "servers/display/display_server.h"
#include "servers/display/display_server_enums.h"

#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>

// 引擎自带 SDK 可能缺少 Win11 新增的 DWM 属性，缺失时按已知数值补上。
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWCP_DONOTROUND
#define DWMWCP_DONOTROUND 1
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
// 表示「沿用系统配色」的哨兵值。
#ifndef DWMWA_COLOR_DEFAULT
#define DWMWA_COLOR_DEFAULT 0xFFFFFFFF
#endif

namespace {

// 新的窗口过程；在 install() 之前需要先声明。
static LRESULT CALLBACK titlebar_window_proc(HWND p_hwnd, UINT p_message, WPARAM p_wparam, LPARAM p_lparam);

// 取可缩放边框的厚度（物理像素），用于把窗口边缘一圈判成缩放区。
// **不用 GetSystemMetricsForDpi**：该 API 要求 Windows 10 1607 以上的 SDK 声明，
// 引擎允许的最小目标版本更低；GetSystemMetrics 在 DPI 感知进程内已按当前 DPI 返回。
static int get_system_frame_thickness() {
	const int l_frame = GetSystemMetrics(SM_CXSIZEFRAME);
	const int l_padded = GetSystemMetrics(SM_CXPADDEDBORDER);
	// 至少留 4px，否则高 DPI 下边框退化成 0，边缘就抓不住了。
	return MAX(l_frame + l_padded, 4);
}

// 新的窗口过程：先交本模块处理，未拦截的消息一律回原窗口过程。
static LRESULT CALLBACK titlebar_window_proc(HWND p_hwnd, UINT p_message, WPARAM p_wparam, LPARAM p_lparam) {
	CustomTitlebar *l_titlebar = CustomTitlebar::get_singleton();
	if (l_titlebar != nullptr) {
		int64_t l_result = 0;
		if (l_titlebar->_handle_window_message((int64_t)p_message, (int64_t)p_wparam,
					(int64_t)p_lparam, l_result)) {
			return (LRESULT)l_result;
		}

		WNDPROC l_original = (WNDPROC)l_titlebar->_get_original_proc();
		if (l_original != nullptr) {
			return CallWindowProcW(l_original, p_hwnd, p_message, p_wparam, p_lparam);
		}
	}

	return DefWindowProcW(p_hwnd, p_message, p_wparam, p_lparam);
}

} // namespace

CustomTitlebar *CustomTitlebar::singleton = nullptr;

CustomTitlebar::CustomTitlebar() {
	singleton = this;
}

CustomTitlebar::~CustomTitlebar() {
	uninstall();
	singleton = nullptr;
}

void CustomTitlebar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_supported"), &CustomTitlebar::is_supported);
	ClassDB::bind_method(D_METHOD("install"), &CustomTitlebar::install);
	ClassDB::bind_method(D_METHOD("uninstall"), &CustomTitlebar::uninstall);
	ClassDB::bind_method(D_METHOD("is_installed"), &CustomTitlebar::is_installed);
	ClassDB::bind_method(D_METHOD("get_caption_height"), &CustomTitlebar::get_caption_height);
	ClassDB::bind_method(D_METHOD("set_caption_height", "height"), &CustomTitlebar::set_caption_height);
	ClassDB::bind_method(D_METHOD("get_border_thickness"), &CustomTitlebar::get_border_thickness);
	ClassDB::bind_method(D_METHOD("set_border_thickness", "thickness"), &CustomTitlebar::set_border_thickness);
	ClassDB::bind_method(D_METHOD("get_interactive_rects"), &CustomTitlebar::get_interactive_rects);
	ClassDB::bind_method(D_METHOD("set_interactive_rects", "rects"), &CustomTitlebar::set_interactive_rects);
	ClassDB::bind_method(D_METHOD("set_border_color", "color"), &CustomTitlebar::set_border_color);
	ClassDB::bind_method(D_METHOD("set_rounded_corners", "rounded"), &CustomTitlebar::set_rounded_corners);
	ClassDB::bind_method(D_METHOD("set_dark_appearance", "dark"), &CustomTitlebar::set_dark_appearance);
	ClassDB::bind_method(D_METHOD("is_maximized"), &CustomTitlebar::is_maximized);
	ClassDB::bind_method(D_METHOD("hide_window"), &CustomTitlebar::hide_window);
	ClassDB::bind_method(D_METHOD("show_window"), &CustomTitlebar::show_window);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "caption_height"), "set_caption_height", "get_caption_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "border_thickness"), "set_border_thickness", "get_border_thickness");
}

bool CustomTitlebar::is_supported() const {
	return true;
}

bool CustomTitlebar::install() {
	if (window_handle != nullptr) {
		return true;
	}

	DisplayServer *l_display = DisplayServer::get_singleton();
	if (l_display == nullptr) {
		return false;
	}

	const int64_t l_handle = l_display->window_get_native_handle(
			DisplayServerEnums::WINDOW_HANDLE, DisplayServerEnums::MAIN_WINDOW_ID);
	if (l_handle == 0) {
		return false;
	}

	HWND l_hwnd = (HWND)l_handle;

	original_proc = (void *)SetWindowLongPtrW(l_hwnd, GWLP_WNDPROC, (LONG_PTR)&titlebar_window_proc);
	if (original_proc == nullptr) {
		return false;
	}

	window_handle = (void *)l_hwnd;

	// 触发一次客户区重算，让新的 WM_NCCALCSIZE 结果立刻生效。
	SetWindowPos(l_hwnd, nullptr, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	return true;
}

void CustomTitlebar::uninstall() {
	if (window_handle == nullptr) {
		return;
	}

	HWND l_hwnd = (HWND)window_handle;

	if (original_proc != nullptr) {
		SetWindowLongPtrW(l_hwnd, GWLP_WNDPROC, (LONG_PTR)original_proc);
	}
	original_proc = nullptr;

	// 触发一次客户区重算，把标题栏还给系统。
	SetWindowPos(l_hwnd, nullptr, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	window_handle = nullptr;
}

bool CustomTitlebar::_is_installed() const {
	return window_handle != nullptr;
}

bool CustomTitlebar::is_maximized() const {
	if (window_handle == nullptr) {
		return false;
	}
	return IsZoomed((HWND)window_handle) != FALSE;
}

bool CustomTitlebar::hide_window() {
	if (window_handle == nullptr) {
		return false;
	}
	// **SW_HIDE 才是真隐藏**：窗口会同时退出任务栏与 Alt-Tab。
	// 「最小化」只是把窗口挪到屏幕外，任务栏里仍然留着，与「只剩托盘」不是一回事。
	ShowWindow((HWND)window_handle, SW_HIDE);
	return true;
}

bool CustomTitlebar::show_window() {
	if (window_handle == nullptr) {
		return false;
	}
	HWND l_hwnd = (HWND)window_handle;

	// **顺序固定**：先让窗口可见并恢复普通态，再抢前台。
	// 隐藏态直接 SetForegroundWindow 是无效的，Windows 不允许激活不可见窗口。
	ShowWindow(l_hwnd, SW_SHOW);
	ShowWindow(l_hwnd, SW_RESTORE);

	// 抢前台的三件套：SetForegroundWindow 单独用常被系统拒绝。
	SetForegroundWindow(l_hwnd);
	BringWindowToTop(l_hwnd);
	SetActiveWindow(l_hwnd);
	return true;
}

void CustomTitlebar::set_caption_height(float p_height) {
	caption_height = MAX(p_height, 0.0f);
}

void CustomTitlebar::set_border_thickness(float p_thickness) {
	border_thickness = MAX(p_thickness, 0.0f);
}

void CustomTitlebar::set_interactive_rects(const Array &p_rects) {
	interactive_rects = p_rects.duplicate();
}

bool CustomTitlebar::_handle_window_message(int64_t p_message, int64_t p_wparam, int64_t p_lparam, int64_t &r_result) {
	if (window_handle == nullptr || original_proc == nullptr) {
		return false;
	}

	HWND l_hwnd = (HWND)window_handle;

	switch (p_message) {
		// 客户区计算：把标题栏所占的高度并入客户区，系统便不再绘制它。
		// **只改尺寸计算，不碰窗口样式**：拖动、缩放、贴边分屏、圆角、双击最大化
		// 全部继续由系统按 WS_OVERLAPPEDWINDOW 处理。
		case WM_NCCALCSIZE: {
			// **只有 wParam 为 TRUE 这条路径能改客户区**，FALSE 时 lParam 是单个 RECT、
			// 没有窗口矩形可参照，交回系统最安全。
			if (p_wparam != TRUE) {
				return false;
			}

			NCCALCSIZE_PARAMS *l_params = (NCCALCSIZE_PARAMS *)p_lparam;
			if (l_params == nullptr) {
				return false;
			}

			// **最小化与隐藏时不介入**：最小化时窗口在屏幕外（-32000），隐藏时窗口
			// 不参与合成，两种状态下按它反推都会得到无效客户区，恢复后就是一块白板。
			if (IsIconic(l_hwnd) || !IsWindowVisible(l_hwnd)) {
				return false;
			}

			// 最大化：窗口矩形比工作区大出一圈边框，直接对齐工作区才是可见区域。
			if (IsZoomed(l_hwnd)) {
				HMONITOR l_monitor = MonitorFromWindow(l_hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO l_info = {};
				l_info.cbSize = sizeof(l_info);
				if (GetMonitorInfoW(l_monitor, &l_info)) {
					l_params->rgrc[0] = l_info.rcWork;
					r_result = 0;
					return true;
				}
				return false;
			}

			// 普通态：**原样返回 0，一个字段都不改**。
			// `rgrc[0]` 进入时就是本次的窗口矩形，返回 0 即「客户区 = 整个窗口」，
			// 标题栏与边框自然消失，而样式仍是 WS_OVERLAPPEDWINDOW，
			// 缩放、贴边分屏、圆角、阴影、最大化动画全部由系统继续提供。
			// **绝不能在这上面做加减**：任何改动都会让客户区越出窗口边界，
			// 系统随即按越界量整体平移内容——表现正是「所有点击位置整体上偏」。
			r_result = 0;
			return true;
		}

		// 命中测试：客户区已铺满整个窗口，系统不再有非客户区可命中，
		// 故缩放边与标题栏必须由本类自己给出命中码。
		case WM_NCHITTEST: {
			// 保留系统的鼠标穿透语义（透明窗口用），其余一律自己判。
			const LRESULT l_default = CallWindowProcW((WNDPROC)original_proc, l_hwnd, WM_NCHITTEST,
					(WPARAM)p_wparam, (LPARAM)p_lparam);
			if (l_default == HTTRANSPARENT) {
				r_result = (int64_t)l_default;
				return true;
			}

			// 最大化时不可缩放，也没有可拖动的标题栏，交给系统即可。
			if (IsIconic(l_hwnd) || IsZoomed(l_hwnd) || !IsWindowVisible(l_hwnd)) {
				return false;
			}

			POINT l_client = { GET_X_LPARAM(p_lparam), GET_Y_LPARAM(p_lparam) };
			if (!ScreenToClient(l_hwnd, &l_client)) {
				return false;
			}

			RECT l_client_rect = {};
			if (!GetClientRect(l_hwnd, &l_client_rect)) {
				return false;
			}

			const int l_frame = MAX(get_system_frame_thickness(), (int)Math::round(border_thickness));
			const bool l_on_left = l_client.x < l_frame;
			const bool l_on_right = l_client.x >= l_client_rect.right - l_frame;
			const bool l_on_top = l_client.y < l_frame;
			const bool l_on_bottom = l_client.y >= l_client_rect.bottom - l_frame;

			// 四角优先：角落同时压在两条边上，先判它才不会被单边抢走。
			if (l_on_top && l_on_left) {
				r_result = (int64_t)HTTOPLEFT;
				return true;
			}
			if (l_on_top && l_on_right) {
				r_result = (int64_t)HTTOPRIGHT;
				return true;
			}
			if (l_on_bottom && l_on_left) {
				r_result = (int64_t)HTBOTTOMLEFT;
				return true;
			}
			if (l_on_bottom && l_on_right) {
				r_result = (int64_t)HTBOTTOMRIGHT;
				return true;
			}
			if (l_on_left) {
				r_result = (int64_t)HTLEFT;
				return true;
			}
			if (l_on_right) {
				r_result = (int64_t)HTRIGHT;
				return true;
			}
			if (l_on_top) {
				r_result = (int64_t)HTTOP;
				return true;
			}
			if (l_on_bottom) {
				r_result = (int64_t)HTBOTTOM;
				return true;
			}

			// 以下才是标题栏内部：只认 [0, caption_height) 这一段。
			if (caption_height <= 0.0f || l_client.y >= (int)Math::round(caption_height)) {
				return false;
			}

			// 标题栏里的按钮要能接收点击，不能被当成拖动区。
			for (int i = 0; i < interactive_rects.size(); i++) {
				const Rect2 l_rect = interactive_rects[i];
				const int l_left = (int)Math::round(l_rect.position.x);
				const int l_right = (int)Math::round(l_rect.position.x + l_rect.size.x);
				const int l_top_edge = (int)Math::round(l_rect.position.y);
				const int l_bottom = (int)Math::round(l_rect.position.y + l_rect.size.y);
				if (l_client.x >= l_left && l_client.x < l_right &&
						l_client.y >= l_top_edge && l_client.y < l_bottom) {
					r_result = (int64_t)HTCLIENT;
					return true;
				}
			}

			r_result = (int64_t)HTCAPTION;
			return true;
		}

		default:
			return false;
	}
}

void CustomTitlebar::set_border_color(const Color &p_color) {
	if (window_handle == nullptr) {
		return;
	}

	COLORREF l_color = DWMWA_COLOR_DEFAULT;
	if (p_color.a > 0.0f) {
		l_color = RGB((BYTE)Math::round(p_color.r * 255.0f),
				(BYTE)Math::round(p_color.g * 255.0f),
				(BYTE)Math::round(p_color.b * 255.0f));
	}
	DwmSetWindowAttribute((HWND)window_handle, DWMWA_BORDER_COLOR, &l_color, sizeof(l_color));
}

void CustomTitlebar::set_rounded_corners(bool p_rounded) {
	if (window_handle == nullptr) {
		return;
	}
	// **必须用显式的 DWMWCP_ROUND，不能用 DWMWCP_DEFAULT**：
	// DEFAULT 是「系统按边框自行决定」，而本模块已把标题栏并入客户区，
	// 帧重算（install 的 SWP_FRAMECHANGED、首次尺寸变化）后系统会判定
	// 窗口无标准边框可圆——表现为启动瞬间有圆角、随后消失。
	// 偏好一经显式设置即持久，最大化/还原不会清除。
	DWORD l_preference = p_rounded ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
	DwmSetWindowAttribute((HWND)window_handle, DWMWA_WINDOW_CORNER_PREFERENCE,
			&l_preference, sizeof(l_preference));
}

void CustomTitlebar::set_dark_appearance(bool p_dark) {
	if (window_handle == nullptr) {
		return;
	}
	BOOL l_dark = p_dark ? TRUE : FALSE;
	DwmSetWindowAttribute((HWND)window_handle, DWMWA_USE_IMMERSIVE_DARK_MODE,
			&l_dark, sizeof(l_dark));
}

#endif // WINDOWS_ENABLED
