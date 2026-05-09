# LyraAnim Editor Debugger 实施计划

> **给 agentic worker：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行。本计划用 checkbox 追踪进度。

**目标：** 在 `LyraAnimEditor` 模块内实现一个 Editor-only 独立窗口，用于在 PIE 时选择 Actor，查看其主 ABP 与 Linked Anim Layer 的蓝图变量，并记录指定变量变化。

**架构：** `LyraAnimEditor` 注册一个 `NomadTab`，窗口由 Slate 实现，分为左侧 Actor 树、中间属性树、右侧记录日志。所有 Editor UI、扫描、反射、记录逻辑都放在 `LyraAnimEditor`，不增加 `LyraAnim` Runtime 依赖。

**技术栈：** UE 5.7 C++、Slate、`FGlobalTabmanager`、`ToolMenus`、`FEditorDelegates`、`FActorIterator`、`UAnimInstance` Reflection、`GEditorPerProjectIni`。

---

## 文件结构

- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditor.Build.cs`
  - 增加 Editor tab/menu 所需依赖。
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditorModule.h`
  - 增加 tab 注册、菜单注册、窗口打开接口。
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditorModule.cpp`
  - 注册 `Lyra Anim Debugger` 菜单与 `NomadTab`。
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerTypes.h`
  - Actor 树、属性项、日志项数据结构。
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerViewModel.h/.cpp`
  - World 扫描、Actor/Mesh/AnimInstance 选择、属性反射、变化记录、设置持久化。
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/SLAAnimDebuggerWindow.h/.cpp`
  - Slate 窗口、左侧树、中间属性折叠区、右侧日志区。

---

## 任务 1：补齐 Build 依赖

**文件：**
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditor.Build.cs`

- [ ] **步骤 1：增加依赖**

在 `PrivateDependencyModuleNames` 中保留已有依赖，并增加：

```csharp
"InputCore",
"UnrealEd",
"ToolMenus",
"EditorStyle",
"WorkspaceMenuStructure",
"Projects"
```

目标依赖块应包含：

```csharp
PrivateDependencyModuleNames.AddRange(
	new string[]
	{
		"CoreUObject",
		"Engine",
		"Slate",
		"SlateCore",
		"InputCore",
		"UnrealEd",
		"ToolMenus",
		"EditorStyle",
		"WorkspaceMenuStructure",
		"Projects",
		"LyraAnim",
		"AnimationModifierLibrary",
		"AnimationLocomotionLibraryEditor",
		"AnimationBlueprintLibrary"
	}
);
```

- [ ] **步骤 2：编译验证**

运行：

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

预期：输出 `Result: Succeeded`。

---

## 任务 2：注册 Editor 菜单与独立窗口

**文件：**
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditorModule.h`
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/LyraAnimEditorModule.cpp`
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/SLAAnimDebuggerWindow.h`
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/SLAAnimDebuggerWindow.cpp`

- [ ] **步骤 1：扩展模块头文件**

`LyraAnimEditorModule.h` 增加：

```cpp
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUICommandList;
class SDockTab;
class FSpawnTabArgs;

class FLyraAnimEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OpenDebuggerTab();
	TSharedRef<SDockTab> SpawnDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<FUICommandList> PluginCommands;
};
```

- [ ] **步骤 2：注册 `NomadTab` 与菜单**

`LyraAnimEditorModule.cpp` 实现：

```cpp
#include "LyraAnimEditorModule.h"

#include "Debug/SLAAnimDebuggerWindow.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

static const FName LyraAnimDebuggerTabName(TEXT("LyraAnimDebugger"));

#define LOCTEXT_NAMESPACE "FLyraAnimEditorModule"

void FLyraAnimEditorModule::StartupModule()
{
	PluginCommands = MakeShared<FUICommandList>();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		LyraAnimDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FLyraAnimEditorModule::SpawnDebuggerTab))
		.SetDisplayName(LOCTEXT("LyraAnimDebuggerTabTitle", "Lyra Anim Debugger"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLyraAnimEditorModule::RegisterMenus));
}

void FLyraAnimEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LyraAnimDebuggerTabName);
	PluginCommands.Reset();
}

void FLyraAnimEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("WindowLayout"));
	Section.AddMenuEntry(
		TEXT("LyraAnimDebugger"),
		LOCTEXT("LyraAnimDebuggerMenuLabel", "Lyra Anim Debugger"),
		LOCTEXT("LyraAnimDebuggerMenuTooltip", "Open the Lyra Anim Debugger window."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FLyraAnimEditorModule::OpenDebuggerTab)));
}

void FLyraAnimEditorModule::OpenDebuggerTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LyraAnimDebuggerTabName);
}

TSharedRef<SDockTab> FLyraAnimEditorModule::SpawnDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLAAnimDebuggerWindow)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLyraAnimEditorModule, LyraAnimEditor)
```

- [ ] **步骤 3：新增临时窗口骨架**

`SLAAnimDebuggerWindow.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SLAAnimDebuggerWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLAAnimDebuggerWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
```

`SLAAnimDebuggerWindow.cpp`：

```cpp
#include "Debug/SLAAnimDebuggerWindow.h"

#include "Widgets/Text/STextBlock.h"

void SLAAnimDebuggerWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Lyra Anim Debugger")))
	];
}
```

- [ ] **步骤 4：编译与手动验证**

运行 Build.bat，预期 `Result: Succeeded`。

打开 Editor，点击 `Window -> Lyra Anim Debugger`。

预期：出现可 Dock / 可拖出的独立 tab，显示 `Lyra Anim Debugger`。

---

## 任务 3：新增数据结构

**文件：**
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerTypes.h`

- [ ] **步骤 1：创建数据结构头文件**

```cpp
#pragma once

#include "CoreMinimal.h"

class AActor;
class UAnimInstance;
class USkeletalMeshComponent;

enum class ELAAnimDebuggerTreeItemType : uint8
{
	Actor,
	Mesh,
	AnimInstance
};

struct FLAAnimDebuggerTreeItem : public TSharedFromThis<FLAAnimDebuggerTreeItem>
{
	ELAAnimDebuggerTreeItemType Type = ELAAnimDebuggerTreeItemType::Actor;
	FString DisplayName;
	TWeakObjectPtr<AActor> Actor;
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TArray<TSharedPtr<FLAAnimDebuggerTreeItem>> Children;
};

struct FLAAnimDebuggerPropertyItem
{
	FString Key;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString ValueText;
	FLinearColor TypeColor = FLinearColor::Gray;
	bool bIsRecorded = false;
};

struct FLAAnimDebuggerLogItem
{
	double TimeSeconds = 0.0;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString OldValue;
	FString NewValue;
};
```

- [ ] **步骤 2：编译验证**

运行 Build.bat，预期 `Result: Succeeded`。

---

## 任务 4：实现 ViewModel 扫描与选择

**文件：**
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerViewModel.h`
- 新增：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerViewModel.cpp`

- [ ] **步骤 1：定义 ViewModel API**

`LAAnimDebuggerViewModel.h` 负责这些接口：

```cpp
class FLAAnimDebuggerViewModel : public TSharedFromThis<FLAAnimDebuggerViewModel>
{
public:
	FLAAnimDebuggerViewModel();
	~FLAAnimDebuggerViewModel();

	void RefreshActorTree();
	const TArray<TSharedPtr<FLAAnimDebuggerTreeItem>>& GetRootItems() const;

	void SetSelectedItem(TSharedPtr<FLAAnimDebuggerTreeItem> InItem);
	TSharedPtr<FLAAnimDebuggerTreeItem> GetSelectedItem() const;

	void RebuildProperties();
	const TArray<FLAAnimDebuggerPropertyItem>& GetProperties() const;

	void SetSearchText(const FString& InSearchText);
	void SetRefreshInterval(float InRefreshInterval);
	float GetRefreshInterval() const;

	void SetFollowEditorSelection(bool bInFollowEditorSelection);
	bool IsFollowingEditorSelection() const;
	bool TryFollowEditorSelection();

	void SetRecorded(const FString& Key, bool bRecorded);
	bool IsRecorded(const FString& Key) const;

	void ClearLog();
	const TArray<TSharedPtr<FLAAnimDebuggerLogItem>>& GetLogs() const;

	void Tick(double CurrentTimeSeconds);
	int32 GetPropertiesRevision() const;
};
```

- [ ] **步骤 2：实现 World 查找**

优先找 PIE World：

```cpp
UWorld* FLAAnimDebuggerViewModel::FindDebugWorld() const
{
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				return Context.World();
			}
		}
	}

	return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}
```

- [ ] **步骤 3：实现 Actor 树扫描**

逻辑：

```cpp
void FLAAnimDebuggerViewModel::RefreshActorTree()
{
	RootItems.Reset();

	UWorld* World = FindDebugWorld();
	if (!World)
	{
		SelectedItem.Reset();
		Properties.Reset();
		return;
	}

	for (FActorIterator It(World); It; ++It)
	{
		AddActorIfValid(*It);
	}

	if (!SelectedItem.IsValid() && RootItems.Num() > 0)
	{
		SelectedItem = RootItems[0];
	}
}
```

Actor 节点只显示至少包含一个有效 `AnimInstance` 的 Actor。

- [ ] **步骤 4：实现 Mesh 与 AnimInstance 子节点**

每个 Actor 下扫描 `USkeletalMeshComponent`，每个 Mesh 下加入：

```cpp
UAnimInstance* MainInstance = Mesh->GetAnimInstance();
TArray<UAnimInstance*> Instances;
Instances.Add(MainInstance);
MainInstance->GetLinkedAnimGraphInstances(Instances);
```

节点命名：

```text
Main ABP: <ClassName>
Linked Layer: <ClassName>
```

- [ ] **步骤 5：实现选择范围**

选择规则：

- 选 Actor：中间显示该 Actor 下所有 Mesh 的所有 AnimInstance。
- 选 Mesh：中间显示该 Mesh 的主 ABP 与 Linked Anim Layer。
- 选 AnimInstance：中间只显示该实例。

- [ ] **步骤 6：实现 `Follow Editor Selection`**

逻辑：

```cpp
USelection* Selection = GEditor->GetSelectedActors();
```

开启后读取第一个有效 Actor，并在左侧树中选中它。若用户手动点击左侧树，可覆盖当前选择。

- [ ] **步骤 7：编译验证**

运行 Build.bat，预期 `Result: Succeeded`。

---

## 任务 5：实现属性反射与变化记录

**文件：**
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/LAAnimDebuggerViewModel.cpp`

- [ ] **步骤 1：遍历属性**

对每个目标 `UAnimInstance`：

```cpp
for (TFieldIterator<FProperty> It(Instance->GetClass()); It; ++It)
{
	FProperty* Property = *It;
	if (!ShouldDisplayProperty(Property))
	{
		continue;
	}

	FLAAnimDebuggerPropertyItem Item;
	Item.InstanceName = Instance->GetClass()->GetName();
	Item.Category = Property->GetMetaData(TEXT("Category"));
	if (Item.Category.IsEmpty())
	{
		Item.Category = TEXT("Default");
	}
	Item.PropertyName = Property->GetName();
	Item.Key = FString::Printf(TEXT("%s:%s"), *Instance->GetPathName(), *Property->GetName());
	Item.ValueText = GetPropertyValueText(Property, Instance);
	Item.TypeColor = GetPropertyTypeColor(Property);
	Item.bIsRecorded = IsRecorded(Item.Key);
}
```

- [ ] **步骤 2：实现属性过滤**

显示：

- `CPF_BlueprintVisible`
- `CPF_Edit`
- 适合观察的 C++ `UPROPERTY`

跳过：

- Delegate / Multicast Delegate
- Array / Map / Set
- 过大的容器展开

- [ ] **步骤 3：实现值转换**

规则：

- `float` / `double` 用 `%.3f`。
- UObject 引用只显示对象名或 `None`。
- 其他使用 `Property->ExportText_InContainer(...)`。
- 文本超过 160 字符截断并追加 `...`。

- [ ] **步骤 4：实现类型颜色**

颜色规则：

```text
bool -> 红色
int/byte -> 青绿
float/double -> 浅绿
enum -> 紫色
FName/FString/FText -> 粉色
FVector -> 黄绿
FRotator -> 紫粉
FTransform -> 橙色
UObject/class -> 蓝色
其他 -> 灰色
```

- [ ] **步骤 5：实现变化日志**

维护：

```cpp
TMap<FString, FString> LastValues;
TSet<FString> RecordedKeys;
TArray<TSharedPtr<FLAAnimDebuggerLogItem>> Logs;
```

规则：

- 第一次采样只写入 `LastValues`，不产生日志。
- 只有 `RecordedKeys` 内的属性变化才追加日志。
- 日志最多 1000 条，超过删除最旧条目。

- [ ] **步骤 6：实现设置持久化**

写入 `GEditorPerProjectIni`：

- `RefreshInterval`
- `FollowEditorSelection`
- `RecordedKeys`

- [ ] **步骤 7：编译验证**

运行 Build.bat，预期 `Result: Succeeded`。

---

## 任务 6：实现 Slate 窗口 UI

**文件：**
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/SLAAnimDebuggerWindow.h`
- 修改：`LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor/Private/Debug/SLAAnimDebuggerWindow.cpp`

- [ ] **步骤 1：窗口结构**

窗口使用：

```text
SVerticalBox
  Toolbar
  SSplitter 横向
    左：Actor Tree
    中：Property ScrollBox
    右：Log ListView
```

- [ ] **步骤 2：顶部工具栏**

保留轻量控制：

- `Refresh Actors`
- `Follow Editor Selection`
- SearchBox
- Update Rate SpinBox
- `Clear Log`

不做 Actor/Mesh 下拉栏。

- [ ] **步骤 3：左侧 Actor 树**

用：

```cpp
STreeView<TSharedPtr<FLAAnimDebuggerTreeItem>>
```

数据源：

```cpp
ViewModel->GetRootItems()
```

选择变化：

```cpp
ViewModel->SetSelectedItem(Item);
RebuildPropertyPanel();
```

- [ ] **步骤 4：中间属性区**

用 `SScrollBox` + 多个 `SExpandableArea`。

分组 Key：

```text
InstanceName / Category
```

每行显示：

```text
[颜色点] PropertyName    ValueText    [Record]
```

- [ ] **步骤 5：右侧日志区**

用：

```cpp
SListView<TSharedPtr<FLAAnimDebuggerLogItem>>
```

显示格式：

```text
Time  Instance / Category.Property  Old -> New
```

- [ ] **步骤 6：PIE 启动刷新**

窗口构造时注册：

```cpp
FEditorDelegates::PostPIEStarted
```

回调中调用：

```cpp
ViewModel->RefreshActorTree();
ActorTreeView->RequestTreeRefresh();
```

析构时移除 Delegate。

- [ ] **步骤 7：Tick 刷新**

`Tick()` 中调用：

```cpp
ViewModel->Tick(InCurrentTime);
RebuildPropertyPanel();
LogListView->RequestListRefresh();
```

使用 `PropertiesRevision` 避免每帧重建 UI。

- [ ] **步骤 8：编译验证**

运行 Build.bat，预期 `Result: Succeeded`。

---

## 任务 7：手动验证与修正

**文件：**
- 按验证结果修改 `LyraAnimEditor/Private/Debug/*`。

- [ ] **步骤 1：最终编译**

运行：

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

预期：`Result: Succeeded`。

- [ ] **步骤 2：Editor 验证**

检查：

- `Window -> Lyra Anim Debugger` 可打开。
- Tab 可拖出 Dock，可放副屏。
- PIE 开始后左侧出现有 `AnimInstance` 的 Actor。
- 点击 Actor，中间显示该 Actor 下所有 ABP 属性。
- 点击 Mesh，中间只显示该 Mesh 的 ABP 属性。
- 点击某个 ABP，中间只显示该 ABP 属性。
- Category 作为折叠区域，不是表格列。
- 类型不显示文字，用颜色点区分。
- 勾选 `Record` 后，右侧只记录该变量变化。
- 第一次采样不产生日志。
- `Clear Log` 清空日志，不取消 `Record`。
- 开启 `Follow Editor Selection` 后，选中 Actor 自动切换观察对象。
- 停止 PIE 不崩溃。

- [ ] **步骤 3：检查 diff**

运行：

```powershell
git status --short
git diff -- "LocomotionTemplate/Plugins/LyraAnim/Source/LyraAnimEditor" "docs/superpowers/specs/2026-05-09-lyraanim-editor-debugger-design.md" "docs/superpowers/plans/2026-05-09-lyraanim-editor-debugger.md"
```

预期：只有计划内文件变化。

---

## 自检

- 覆盖 spec：Editor-only、独立窗口、左侧 Actor 树、Actor/Mesh/ABP 选择、中间 Category 折叠、类型颜色、右侧记录日志、`Follow Editor Selection`、设置持久化、编译与手动验证均覆盖。
- 范围控制：第一版不做变量编辑、视口 Overlay、Shipping 支持、时间线图表、日志导出、复杂容器展开。
- 风险点：Slate UI 每次重建可能较重，因此计划要求 `PropertiesRevision` 限制重建频率。
- 验证路径：每个主要任务后都用项目指定 Build.bat 编译，最终做 Editor 手动验证。
