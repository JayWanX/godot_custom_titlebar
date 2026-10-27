# CustomTitlebar

继承自：[Object](https://docs.godotengine.org/en/stable/classes/class_object.html)

自定义标题栏：让窗口保用系统原生边框（拖动、缩放、贴边分屏、圆角、阴影全部由系统提供），

但把标题栏区域并入客户区，由 GDScript 在客户区顶部自绘。

## 描述：

**不动窗口样式，只改客户区计算**：接管 `WM_NCCALCSIZE` 把客户区扩到整个窗口矩形，

系统就不再绘制标题栏，而 `WS_THICKFRAME` 仍归系统所有——因此缩放边框、Aero Snap、

双击最大化、最小化/最大化动画全部保持原生行为，不经 GDScript 逐帧算坐标。

**样式必须保持原样**：引擎的 `_update_window_style()` 会在切换窗口模式时按

`WS_OVERLAPPEDWINDOW` 整块重写 `GWL_STYLE`，改过的样式会被覆盖回去。

仅在 Windows 上可用。

## 属性：


类型                                                                        | 名称                                       | 默认值 
------------------------------------------------------------------------- | ---------------------------------------- | ----
[float](https://docs.godotengine.org/en/stable/classes/class_float.html)  | [border_thickness](#i_border_thickness)  | 8.0 
[float](https://docs.godotengine.org/en/stable/classes/class_float.html)  | [caption_height](#i_caption_height)      | 0.0 
<p></p>

## 方法：


返回值                                                                       | 函数签名                                                                                                                                  
------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------
[Array](https://docs.godotengine.org/en/stable/classes/class_array.html)  | [get_interactive_rects](#i_get_interactive_rects) ( ) const                                                                           
[bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)    | [install](#i_install) ( )                                                                                                             
[bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)    | [is_installed](#i_is_installed) ( ) const                                                                                             
[bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)    | [is_maximized](#i_is_maximized) ( ) const                                                                                             
[bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)    | [is_supported](#i_is_supported) ( ) const                                                                                             
[void](#)                                                                 | [set_border_color](#i_set_border_color) ( [Color](https://docs.godotengine.org/en/stable/classes/class_color.html) color )            
[void](#)                                                                 | [set_dark_appearance](#i_set_dark_appearance) ( [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html) dark )         
[void](#)                                                                 | [set_interactive_rects](#i_set_interactive_rects) ( [Array](https://docs.godotengine.org/en/stable/classes/class_array.html) rects )  
[void](#)                                                                 | [set_rounded_corners](#i_set_rounded_corners) ( [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html) rounded )      
[void](#)                                                                 | [uninstall](#i_uninstall) ( )                                                                                                         
<p></p>

## 属性描述

### [float](https://docs.godotengine.org/en/stable/classes/class_float.html)<span id="i_border_thickness"></span> **border_thickness** = 8.0

客户区顶部留给缩放边框的高度，避免顶边无法拖拽调整大小。

### [float](https://docs.godotengine.org/en/stable/classes/class_float.html)<span id="i_caption_height"></span> **caption_height** = 0.0

标题栏高度，客户区坐标下的像素值。

## 方法描述

### [Array](https://docs.godotengine.org/en/stable/classes/class_array.html)<span id="i_get_interactive_rects"></span> **get_interactive_rects**( ) 

取标题栏内的交互区域。

### [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)<span id="i_install"></span> **install**( ) 

安装：替换窗口过程并把标题栏并入客户区。

返回 false 表示系统不支持或取不到窗口句柄。

### [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)<span id="i_is_installed"></span> **is_installed**( ) 

当前是否已安装。

### [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)<span id="i_is_maximized"></span> **is_maximized**( ) 

窗口当前是否处于最大化。

### [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html)<span id="i_is_supported"></span> **is_supported**( ) 

当前平台是否支持自定义标题栏。

### [void](#)<span id="i_set_border_color"></span> **set_border_color**( [Color](https://docs.godotengine.org/en/stable/classes/class_color.html) color ) 

设置窗口边框色。

**color：** 边框颜色；alpha 为 0 表示沿用系统默认。

### [void](#)<span id="i_set_dark_appearance"></span> **set_dark_appearance**( [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html) dark ) 

设置窗口深浅色外观，影响系统绘制的边框、阴影与右键菜单配色。

**dark：** 为 true 使用深色外观。

### [void](#)<span id="i_set_interactive_rects"></span> **set_interactive_rects**( [Array](https://docs.godotengine.org/en/stable/classes/class_array.html) rects ) 

设置标题栏内需要接收鼠标的区域（按钮等）。

落在这些矩形内的位置不会报成标题栏，点击才会交给界面控件。

**rects：** 矩形数组，客户区坐标。

### [void](#)<span id="i_set_rounded_corners"></span> **set_rounded_corners**( [bool](https://docs.godotengine.org/en/stable/classes/class_bool.html) rounded ) 

设置窗口圆角。

**rounded：** 为 true 使用系统圆角，false 使用直角。

### [void](#)<span id="i_uninstall"></span> **uninstall**( ) 

卸载：恢复原生标题栏与原始窗口过程。

_生成于 2026-10-27_
