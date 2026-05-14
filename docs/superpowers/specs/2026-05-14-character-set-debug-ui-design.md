# CharacterSet DebugUI 扩展设计

## 概述

扩展 `ULTCharacterSet`，让每个角色配置自己在 DebugPanel 中显示哪些动画开关，并提供一段角色说明文字。该配置只影响 DebugPanel 展示，不改变动画系统真实运行开关。

## 目标

- `CharacterSet` 可配置说明文字，DebugPanel 显示当前角色说明。
- `CharacterSet` 可配置每个动画开关是否显示。
- 复用现有 `FLAAnimToggleSettings` 字段，变量命名明确表达 UI 可见性用途。
- 不修改 `LyraAnim` 插件逻辑。
- 默认保持旧行为：未配置时 DebugPanel 仍显示现有全部开关。

## 数据设计

在 `ULTCharacterSet` 中新增字段：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
FText Description;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug UI")
FLAAnimToggleSettings DebugToggleVisibility;
```

字段语义：

- `Description`：当前角色说明文字，仅用于 DebugPanel 展示。
- `DebugToggleVisibility`：DebugPanel 动画开关可见性配置。
- `DebugToggleVisibility.bEnableStart == false` 表示隐藏“起步动画”开关。
- `DebugToggleVisibility.bEnableFootIK == false` 表示隐藏“脚步IK”开关。
- `DebugToggleVisibility` 不作为动画功能开关，不写入 `ULASettings::ToggleSettings`。

`FLAAnimToggleSettings::bUseBlendSpaceLoop` 默认值是 `false`，但在可见性语义下需要默认显示。`ULTCharacterSet` 构造函数将 `DebugToggleVisibility.bUseBlendSpaceLoop` 初始化为 `true`，保证新字段默认全显示，避免旧资产升级后隐藏 BlendSpace 开关。

## UI 设计

DebugPanel 新增当前角色说明区域：

- 读取当前 `CachedCharacter->GetCurrentCharacterSetIndex()`。
- 从 `ULTProjectSettings::CharacterSets` 加载当前 `ULTCharacterSet`。
- `Description` 非空时显示说明文字。
- `Description` 为空时整段隐藏。

说明区域位置：放在“角色列表”之后、“重置默认值”之前，保持用户先选择角色，再查看该角色说明。

动画开关显示规则：

- 每个 Toggle 行增加可见性判断。
- 当前 `CharacterSet` 存在且对应 `DebugToggleVisibility` 字段为 `false` 时，整行 `Collapsed`。
- 当前 `CharacterSet` 不存在时，按默认全显示处理。
- 原有启用/禁用逻辑保留，例如 BlendSpace 模式下禁用大多数高级开关。
- 原有依赖逻辑保留，例如 Start DM 仍依赖 Start 当前启用。

## 数据流

1. DebugPanel 构建 Toggle 行时绑定动态 Visibility。
2. Visibility Lambda 查询当前 `CharacterSet` 的 `DebugToggleVisibility`。
3. 用户点击角色按钮后 `ALTCharacter::ApplyCharacterSet(Index)` 更新当前索引。
4. Slate 属性下一次刷新时自动根据新角色重算可见性和说明文字。
5. 用户勾选可见 Toggle 时，仍调用现有 `OnToggleCheckChanged` 写入 `ULASettings::ToggleSettings` 和 `ULTDebugSubsystem`。

## 组件变更

| 文件 | 变更 |
|------|------|
| `Source/LocomotionTemplate/Data/LTCharacterSet.h` | 引入 `FLAAnimToggleSettings`，新增 `Description`、`DebugToggleVisibility`、构造函数声明 |
| `Source/LocomotionTemplate/Data/LTCharacterSet.cpp` | 构造函数初始化 `DebugToggleVisibility.bUseBlendSpaceLoop = true` |
| `Source/LocomotionTemplate/UI/SLTDebugPanel.h` | 增加获取当前 `CharacterSet`、说明文字、Toggle 可见性查询函数 |
| `Source/LocomotionTemplate/UI/SLTDebugPanel.cpp` | 为 Toggle 行绑定 Visibility，新增说明区域 |

## 非目标

- 不改 `LyraAnim` 插件。
- 不改变动画开关默认值和持久化格式。
- 不在切换角色时自动重置或覆盖 `ULASettings::ToggleSettings`。
- 不修改 `Content/` 下 DataAsset 二进制资产。

## 验证

- 编译 `LocomotionTemplateEditor Win64 Development` 成功。
- 默认配置下 DebugPanel 仍显示全部现有动画开关。
- 某个 `CharacterSet.DebugToggleVisibility.bEnableFootIK = false` 时，切到该角色后“脚步IK”行隐藏。
- `Description` 为空时不显示说明区域，非空时显示对应文字。
- 隐藏某行后，其真实动画开关值不被修改。
