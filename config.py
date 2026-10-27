# SCons 构建配置：告知引擎本模块的编译条件、公开的文档类及其文档路径。
def can_build(env, platform):
    # 本模块直接操作 Win32 窗口消息与 DWM 属性，只有 Windows 平台可编译。
    return platform == "windows"


def configure(env):
    # 本模块无需额外的构建选项；所需系统库由平台 detect.py 全局链接。
    pass


def get_doc_classes():
    # 需要公开到引擎类参考中的所有类名。
    return [
        "CustomTitlebar",
    ]


def get_doc_path():
    # 类参考文档（XML）所在目录，相对模块根目录。
    return "doc/classes"
