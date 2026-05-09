# LyraAnim Editor Debugger 设计

## 背景

`LyraAnim` 插件内有多个 `AnimBlueprint`、主 `AnimInstance` 和 Linked Anim Layer。调试运动系统时，需要在 PIE 运行期间持续观察 ABP 蓝图变量当前值，以及少量重点变量的变化历史。现有项目 DebugPanel 是游戏内 Overlay，不适合展示大量信息，也不能方便放到副屏幕常驻。

本设计目标是给 `LyraAnim` 插件提供一个 Editor 专用独立窗口，类似 `VegaGASDebugger` 的 `NomadTab` 工具窗口。窗口可以从 `Window` 菜单打开、拖出 Dock、放到副屏幕，在 PIE 期间保持可见。

## 目标

- 在 `LyraAnimEditor` 模块中提供独立 Editor 窗口：`Lyra Anim Debugger`。
- PIE 运行时通过左侧 Actor 树选择场景中的 Actor，并观察该 Actor 下的动画实例。
- 展示该 Mesh 的主 `AnimInstance` 和所有 Linked Anim Layer 实例。
- 读取 ABP 蓝图变量、C++ `UPROPERTY` 变量的当前值。
- 使用 `Category` metadata 作为折叠分组，而不是表格列。
- 不显示变量类型文本，使用接近 Blueprint pin 的颜色表达类型。
- 支持对指定变量启用“记录”，右侧日志只显示这些变量的值变化。
- 工具仅面向 Editor / PIE，不要求 Standalone、Packaged、Shipping 可用。

## 非目标

- 不做游戏视口 Overlay。
- 不在第一版支持修改 ABP 变量值。
- 不记录所有变量变化，只记录用户勾选“记录”的变量。
- 不支持 Shipping 构建。
- 不修改具体项目 DebugPanel。
- 不要求读取未加载资产中的默认值，只观察运行中实例。

## 模块位置

实现放在 `LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/`。

`LyraAnimEditor` 负责：

- 注册 Editor 菜单。
- 注册 `NomadTab`。
- 构建 Slate 独立窗口。
- 在 PIE 世界中扫描 Actor 和 Mesh。
- 通过 Reflection 读取 `AnimInstance` 属性。

`LyraAnim` Runtime 模块第一版不新增依赖。原因：工具只在 Editor 使用，放在 Editor 模块可避免 Runtime 增加 Slate / UnrealEd 依赖。

## 入口与窗口生命周期

参考 `VegaGASDebugger`：

- 在 `FLyraAnimEditorModule::StartupModule()` 中调用 `FGlobalTabmanager::RegisterNomadTabSpawner`。
- 在 `Window` 菜单下增加 `Lyra Anim Debugger`。
- 点击菜单后调用 `FGlobalTabmanager::TryInvokeTab`。
- `OnSpawnPluginTab()` 返回 `SDockTab`，内容为 `SLADebuggerWindow`。
- `SLADebuggerWindow` 构造时注册 `FEditorDelegates::PostPIEStarted`，PIE 启动后自动刷新 Actor/Mesh 列表。
- 析构时移除 Delegate。

窗口是 UE Editor Dock Tab。用户可以把它拖成独立窗口并移动到副屏幕。

## UI 布局

整体使用横向 `SSplitter`：左侧 Actor 树，中间属性区，右侧日志区。

### 顶部工具栏

顶部工具栏只保留轻量控制，不做 `VegaGASDebugger` 那种 Actor / Mesh 刷新栏：

- Search：过滤变量名和 Category。
- Update Rate：刷新间隔，默认 `0.1s`。
- Clear Log：清空右侧日志。
- Follow Editor Selection：跟随当前 Editor 选中的 Actor。开启后，在 PIE 中选中某个 Actor，窗口自动切换观察对象。

### 左侧：Actor 与实例导航

左侧直接按 Actor 展示，不使用顶部 Actor 下拉。每个 Actor 节点下展开它的 `USkeletalMeshComponent` 和动画实例：

```text
BP_Character_C_0
  CharacterMesh0
    Main ABP: ABP_LA_Template_C
    Linked Layer: ABP_LA_Locomotion_C
    Linked Layer: ABP_LA_AimOffset_C

BP_NPC_C_2
  Mesh
    Main ABP: ABP_LA_Template_C
```

点击 Actor 后，中间显示该 Actor 下所有 Mesh 的所有动画实例。点击 Mesh 后，只显示该 Mesh 的动画实例。点击某个 AnimInstance 后，只显示该实例属性。

左侧列表定时轻量刷新，或在 PIE 启动时刷新一次。第一版可提供一个小型刷新按钮放在左侧栏标题处，用于重新扫描 Actor；不使用顶部全局刷新栏。

### 中间：属性树

中间不使用传统表格。结构为：

```text
Main ABP: ABP_LA_Template_C
  Movement
    [绿点] Speed                         342.100      [记录]
    [红点] bIsMoving                     true         [记录]
  State
    [紫点] MovementMode                  Walking      [记录]

Linked Layer: ABP_LA_Locomotion_C
  Start
    [红点] bEnableStart                  true         [记录]
    [绿点] StartDistance                 186.300      [记录]
```

层级：

1. `AnimInstance`
2. `Category`
3. `Property`

`Category` 使用 `SExpandableArea`。空 Category 归入 `Default`。子分类如 `Locomotion|Start` 按原字符串显示，第一版不强制拆成多级树。

每个变量行包含：

- 类型颜色点。
- 变量名。
- 当前值。
- “记录”复选框。

### 类型颜色

不显示类型文本，使用接近 Blueprint pin 的颜色：

| 类型 | 颜色意图 |
|------|----------|
| `bool` | 红色 |
| `int32` / `int64` / byte | 青绿 |
| `float` / `double` | 浅绿 |
| enum | 紫色 |
| `FName` / `FString` / `FText` | 粉色 |
| `FVector` | 黄绿 |
| `FRotator` | 紫粉 |
| UObject / class reference | 蓝色 |
| 其他 Struct | 灰色 |

第一版可用固定 `FLinearColor`，不要求完全匹配引擎 Blueprint pin 主题。

### 右侧：记录日志

右侧只显示被勾选“记录”的属性变化。

日志格式：

```text
12.300  Main ABP / Movement.Speed        320.400 -> 342.100
12.480  Main ABP / Movement.Speed        342.100 -> 356.800
12.510  Main ABP / Movement.bIsMoving    false -> true
```

行为：

- 第一次采样只建立初始值，不写日志。
- 之后只有值发生变化才写日志。
- 日志最多保留 1000 条，超出后移除最旧条目。
- `Clear Log` 清空日志，但不取消已记录变量。

## 数据模型

核心结构建议：

```cpp
struct FLAEditorAnimInstanceItem
{
	TWeakObjectPtr<UAnimInstance> Instance;
	FString DisplayName;
	bool bIsMainInstance = false;
};

struct FLAEditorAnimPropertyItem
{
	FString Key;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString ValueText;
	FLinearColor TypeColor;
	bool bIsRecorded = false;
};

struct FLAEditorAnimPropertyLogItem
{
	double TimeSeconds = 0.0;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString OldValue;
	FString NewValue;
};
```

`Key` 用于跨刷新追踪同一变量：

```text
ObjectPath(AnimInstance) + ":" + PropertyName
```

如果同一蓝图类出现重名属性，后续可扩展为 `Property->GetPathName()`。

## World 与对象扫描

窗口刷新时查找 PIE World：

```cpp
for (const FWorldContext& Context : GEngine->GetWorldContexts())
{
	if (Context.WorldType == EWorldType::PIE)
	{
		World = Context.World();
		break;
	}
}
```

如果没有 PIE World，可以退回 Editor World，但 UI 应提示“未运行 PIE，属性值可能不是运行态”。第一版主要面向 PIE。

Actor 扫描：

- 遍历 `FActorIterator`。
- 查找 Actor 下所有 `USkeletalMeshComponent`。
- 只保留 `Mesh->GetAnimInstance() != nullptr` 的 Mesh。
- 左侧 Actor 树只显示至少有一个有效 AnimInstance 的 Actor。
- 若 `Follow Editor Selection` 开启，优先观察当前 Editor 选中的 Actor；若该 Actor 没有有效 AnimInstance，显示空状态提示。

AnimInstance 扫描：

- `Mesh->GetAnimInstance()` 作为主实例。
- `MainInstance->GetLinkedAnimGraphInstances(LinkedInstances)` 获取 Linked Anim Layer。
- 去重后显示。

## 属性读取

对每个 `UAnimInstance` 使用 Reflection：

```cpp
for (TFieldIterator<FProperty> It(Instance->GetClass()); It; ++It)
{
	FProperty* Property = *It;
	FString Category = Property->GetMetaData(TEXT("Category"));
	FString ValueText;
	Property->ExportText_InContainer(0, ValueText, Instance, Instance, Instance, PPF_None);
}
```

过滤规则：

- 默认显示 `BlueprintVisible`、`BlueprintReadOnly`、`BlueprintReadWrite`、C++ `UPROPERTY` 中适合观察的变量。
- 跳过函数、Delegate、Multicast Delegate。
- 数组、Map、Set 第一版默认跳过，避免大量文本刷屏。
- UObject 引用只显示对象名，不展开对象内部。
- 常见 Struct 白名单：`FVector`、`FRotator`、`FTransform`。
- 其他 Struct 第一版显示 `ExportText` 结果，如过长则截断。

Category 读取：

```cpp
Property->GetMetaData(TEXT("Category"))
```

Editor / PIE 下可读取 Blueprint 变量分类。若为空，归到 `Default`。

## 刷新与变化记录

窗口 `Tick` 中按刷新间隔采样，不每帧全量反射。默认 `0.1s`。

内部维护：

- `LastValues`: `TMap<FString, FString>`，保存上次采样值。
- `RecordedKeys`: `TSet<FString>`，保存勾选“记录”的变量。
- `LogItems`: 右侧日志数据源。

逻辑：

1. 采样当前属性列表。
2. 更新中间属性显示。
3. 对每个 `RecordedKeys` 中的属性比较 `LastValues`。
4. 若旧值存在且新旧不同，追加日志。
5. 更新 `LastValues`。

float / double 文本格式使用固定精度，例如 3 位小数，减少微小抖动导致的日志刷屏。

## Editor 选中 Actor 观察

工具支持直接观察当前选中的 Actor：

- 用户在 PIE viewport 或 World Outliner 中选中 Actor。
- `Follow Editor Selection` 开启时，窗口读取 `GEditor->GetSelectedActors()`。
- 若选中 Actor 有 `USkeletalMeshComponent` 且 Mesh 有 `AnimInstance`，左侧自动选中该 Actor。
- 若选中多个 Actor，第一版只使用第一个有效 Actor。
- 手动点击左侧 Actor 会关闭或覆盖当前跟随选择，避免用户手动选择后被 Editor Selection 抢回。

该模式能满足“选中某个 Actor 直接观察某个 Actor”的工作流。

## 持久化

第一版持久化到 `GEditorPerProjectIni`：

- 上次选中的 Actor 名称。
- 上次选中的 Mesh 名称或 AnimInstance 名称。
- 已记录属性 Key。
- 刷新间隔。
- 是否开启 `Follow Editor Selection`。

若对象路径变化导致 Key 失效，自动忽略，不报错。

## 错误与空状态

- 未 PIE：显示“未检测到 PIE World”。
- 无可用 Mesh：显示“未找到带 AnimInstance 的 SkeletalMeshComponent”。
- 选中对象失效：清空属性区并提示“目标已失效，请 Refresh”。
- 属性读取失败：该属性行显示 `<unreadable>`，不中断刷新。

## 性能约束

- 默认刷新间隔 `0.1s`。
- 不每帧重建全部 Slate Widget；属性值应尽量通过数据源刷新。
- 日志最多 1000 条。
- 搜索过滤在采样后 UI 层处理。
- 大容器类型默认跳过，避免 `ExportText` 生成大量文本。

## 测试要点

- Editor 菜单可打开 `Lyra Anim Debugger`。
- 窗口可拖出 Dock 并在 PIE 期间保持显示。
- PIE 开始后左侧 Actor 树自动刷新。
- 开启 `Follow Editor Selection` 后，选中 Actor 可自动切换观察对象。
- 能显示主 ABP 与 Linked Anim Layer。
- Blueprint 变量按 Category 折叠显示。
- 不显示类型列，类型颜色可区分常见类型。
- 勾选“记录”后，变量变化进入右侧日志。
- 第一次采样不产生日志。
- Clear Log 只清空日志，不取消记录状态。
- 停止 PIE 后不崩溃，失效对象能被安全处理。

## 第一版交付范围

第一版只实现 Editor 独立窗口、对象选择、属性折叠显示、类型颜色、记录日志。变量编辑、时间线图表、导出日志、复杂容器展开不进入第一版。
