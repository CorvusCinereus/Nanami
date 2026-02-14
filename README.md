# Nanami

#### 介绍
使用raylib制作的七实桌宠

#### 编译

```bash
xmake
```

#### 使用说明

1. 右键呼出菜单
2. 按backspace键切换为鼠标跟随模式

#### 配置文件说明

```json
# settings.json
{
    "scale": 0.5, # 桌宠缩放比例
    "font": 2.0, # 字体缩放比例
    "restTime": 30.0, # 提醒休息的间隔时长
    "enableAI": false # 是否启用AI对话功能
}
```

#### AI对话功能说明

需在可执行文件同一目录创建一个`ai.json`文件，url和model需自行配置

```json
{
    "model": "...",
    "url": "..."
}
```

#### 许可证

本项目采用 **GNU General Public License v2.0** 开源协议。

版权所有 (C) 2026 CorvusCinereus

详情请参阅项目根目录的 [COPYING](COPYING) 文件。