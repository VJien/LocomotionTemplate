# DebugPanel 状态持久化 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Editor 每次重新 Play 时，自动恢复上一次 DebugPanel 的所有运行时状态（Toggle开关、CMC参数、CharacterSet选择）。

**Architecture:** 在 `ULTDebugSubsystem`（GameInstanceSubsystem）中增加 JSON 序列化/反序列化。Deinitialize 写入 `Saved/LTDebugState.json`，Initialize 读取还原。`ALTCharacter::BeginPlay` 从 Subsystem 读状态并应用到 CMC/Camera/Strafe/CharacterSet/Toggle。`FLTDebugState` 直接嵌套 `FLAAnimToggleSettings`，后续新增开关只需改插件一处。

**Tech Stack:** UE5 `FJsonObjectConverter`（JSON 序列化），`FPaths::ProjectSavedDir()`（文件路径），`FFileHelper`（IO）。

---

## 需要持久化的数据

| 数据 | 来源 | 当前位置 |
|------|------|----------|
| CMC 四参数 (MaxAcceleration, BrakingDeceleration, MaxWalkSpeed, CameraArmLength) | DebugPanel Slider | `ULTDebugSubsystem::CMCParams` |
| 动画 Toggle（`FLAAnimToggleSettings` 整体） | DebugPanel CheckBox | `ULASettings::ToggleSettings` |
| CharacterSet Index | DebugPanel Button | `ALTCharacter::CurrentCharacterSetIndex` |
| Strafe 模式 | HUD HintBar | `ALTCharacter::bStrafeMovement` |
| Front Camera 状态 | V键切换 | `ALTCharacter::bFrontCameraActive` |

## 存档文件

路径：`FPaths::ProjectSavedDir() / TEXT("LTDebugState.json")`

即 `LocomotionTemplate/Saved/LTDebugState.json`

---

## File Structure

| 文件 | 操作 | 说明 |
|------|------|------|
| `Source/LocomotionTemplate/Character/Locomotion/LTLocomotionTypes.h` | 修改 | 新增 `FLTDebugState` USTRUCT，嵌套 `FLAAnimToggleSettings` |
| `Source/LocomotionTemplate/Core/LTDebugSubsystem.h` | 修改 | 增加 Save/Load/Restore 方法，新增 `FLTDebugState` 成员 |
| `Source/LocomotionTemplate/Core/LTDebugSubsystem.cpp` | 修改 | 实现 JSON 序列化、文件IO |
| `Source/LocomotionTemplate/Character/LTCharacter.cpp` | 修改 | `BeginPlay` 调用 Subsystem 恢复状态 |
| `Source/LocomotionTemplate/LocomotionTemplate.Build.cs` | 修改 | 添加 `Json`, `JsonUtilities` 模块依赖 |

---

### Task 1: Build.cs 添加 JSON 模块依赖

**Files:**
- Modify: `Source/LocomotionTemplate/LocomotionTemplate.Build.cs`

- [ ] **Step 1: 在 PrivateDependencyModuleNames 中添加 JSON 模块**

当前：
```csharp
PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
```

改为：
```csharp
PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "Json", "JsonUtilities" });
```

---

### Task 2: 定义 FLTDebugState 结构体

**Files:**
- Modify: `Source/LocomotionTemplate/Character/Locomotion/LTLocomotionTypes.h`

需要 include `LASettings.h` 以使用 `FLAAnimToggleSettings`。

- [ ] **Step 1: 在 LTLocomotionTypes.h 顶部添加 include**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "LyraAnim/Data/LASettings.h"
#include "LTLocomotionTypes.generated.h"
```

- [ ] **Step 2: 在 `FLTCMCParams` 之后追加 `FLTDebugState`**

```cpp
USTRUCT(BlueprintType)
struct FLTDebugState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FLTCMCParams CMCParams;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FLAAnimToggleSettings ToggleSettings;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	int32 CharacterSetIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	bool bStrafeMovement = true;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	bool bFrontCameraActive = false;
};
```

核心设计：`FLAAnimToggleSettings` 直接嵌套，后续插件新增开关自动被持久化，无需改此处。

- [ ] **Step 3: 编译验证**

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected: `Result: Succeeded`

---

### Task 3: ULTDebugSubsystem 增加 Save/Load/Restore

**Files:**
- Modify: `Source/LocomotionTemplate/Core/LTDebugSubsystem.h`
- Modify: `Source/LocomotionTemplate/Core/LTDebugSubsystem.cpp`

- [ ] **Step 1: 修改 LTDebugSubsystem.h**

替换为：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/Locomotion/LTLocomotionTypes.h"
#include "LTDebugSubsystem.generated.h"

UCLASS()
class LOCOMOTIONTEMPLATE_API ULTDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Locomotion|Debug")
	const FLTCMCParams& GetCMCParams() const;

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Debug")
	void SetCMCParams(const FLTCMCParams& InParams);

	UFUNCTION(BlueprintPure, Category = "Locomotion|Debug")
	const FLTDebugState& GetDebugState() const { return DebugState; }

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Debug")
	void SetDebugState(const FLTDebugState& InState);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Debug")
	void ResetToDefaults();

	void CaptureCurrentState(int32 InCharSetIndex, bool bInStrafe, bool bInFrontCamera);
	void SaveToFile();
	bool LoadFromFile();

private:
	FLTDebugState DebugState;
	FLTDebugState DefaultState;

	static FString GetSaveFilePath();
};
```

- [ ] **Step 2: 修改 LTDebugSubsystem.cpp**

替换为：

```cpp
#include "Core/LTDebugSubsystem.h"
#include "Core/LTProjectSettings.h"
#include "LyraAnim/Data/LASettings.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString ULTDebugSubsystem::GetSaveFilePath()
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LTDebugState.json"));
}

void ULTDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const ULTProjectSettings* PS = ULTProjectSettings::Get())
	{
		DefaultState.CMCParams.MaxAcceleration = PS->MaxAccelerationRange.Default;
		DefaultState.CMCParams.BrakingDeceleration = PS->BrakingDecelerationRange.Default;
		DefaultState.CMCParams.MaxWalkSpeed = PS->MaxWalkSpeedRange.Default;
		DefaultState.CMCParams.CameraArmLength = PS->CameraArmLengthRange.Default;
	}

	DefaultState.ToggleSettings = ULASettings::GetConst()
		? ULASettings::GetConst()->GetToggleSettings()
		: FLAAnimToggleSettings();

	DebugState = DefaultState;

	LoadFromFile();
}

void ULTDebugSubsystem::Deinitialize()
{
	SaveToFile();
	Super::Deinitialize();
}

const FLTCMCParams& ULTDebugSubsystem::GetCMCParams() const
{
	return DebugState.CMCParams;
}

void ULTDebugSubsystem::SetCMCParams(const FLTCMCParams& InParams)
{
	DebugState.CMCParams = InParams;
}

const FLTDebugState& ULTDebugSubsystem::GetDebugState() const
{
	return DebugState;
}

void ULTDebugSubsystem::SetDebugState(const FLTDebugState& InState)
{
	DebugState = InState;
}

void ULTDebugSubsystem::ResetToDefaults()
{
	DebugState = DefaultState;

	if (ULASettings* LAS = ULASettings::Get())
	{
		LAS->SetToggleSettings(DefaultState.ToggleSettings);
	}
}

void ULTDebugSubsystem::CaptureCurrentState(int32 InCharSetIndex, bool bInStrafe, bool bInFrontCamera)
{
	DebugState.CharacterSetIndex = InCharSetIndex;
	DebugState.bStrafeMovement = bInStrafe;
	DebugState.bFrontCameraActive = bInFrontCamera;
}

void ULTDebugSubsystem::SaveToFile()
{
	FString JsonStr;
	if (FJsonObjectConverter::UStructToJsonObjectString(FLTDebugState::StaticStruct(), &DebugState, JsonStr, 0, 0))
	{
		FFileHelper::SaveStringToFile(JsonStr, *GetSaveFilePath(), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

bool ULTDebugSubsystem::LoadFromFile()
{
	const FString FilePath = GetSaveFilePath();
	if (!FPaths::FileExists(FilePath))
	{
		return false;
	}

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		return false;
	}

	FLTDebugState LoadedState;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonStr, &LoadedState, 0, 0))
	{
		DebugState = LoadedState;
		return true;
	}

	return false;
}
```

注意简化点：
- `DefaultState.ToggleSettings` 直接从 `ULASettings` CDO 整体赋值，无需逐字段拷贝
- `ResetToDefaults` 直接 `LAS->SetToggleSettings(DefaultState.ToggleSettings)`，无需逐字段
- 后续 `FLAAnimToggleSettings` 增减字段，此处零改动

- [ ] **Step 3: 编译验证**

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected: `Result: Succeeded`

---

### Task 4: ALTCharacter::BeginPlay 从 Subsystem 恢复状态

**Files:**
- Modify: `Source/LocomotionTemplate/Character/LTCharacter.cpp`

- [ ] **Step 1: 在 LTCharacter.cpp 顶部添加 include**

在已有 include 后追加：

```cpp
#include "Core/LTDebugSubsystem.h"
#include "LyraAnim/Data/LASettings.h"
```

- [ ] **Step 2: 修改 BeginPlay 实现状态恢复**

将 `ALTCharacter::BeginPlay` 从：

```cpp
void ALTCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyCharacterSet(0);
}
```

改为：

```cpp
void ALTCharacter::BeginPlay()
{
	Super::BeginPlay();

	ULTDebugSubsystem* DebugSS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>();
	const FLTDebugState& State = DebugSS->GetDebugState();

	GetCharacterMovement()->MaxAcceleration = State.CMCParams.MaxAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = State.CMCParams.BrakingDeceleration;
	GetCharacterMovement()->MaxWalkSpeed = State.CMCParams.MaxWalkSpeed;
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = State.CMCParams.CameraArmLength;
	}

	bStrafeMovement = State.bStrafeMovement;
	GetCharacterMovement()->bOrientRotationToMovement = !bStrafeMovement;
	bUseControllerRotationYaw = bStrafeMovement;

	ApplyCharacterSet(State.CharacterSetIndex);

	if (ULASettings* LAS = ULASettings::Get())
	{
		LAS->SetToggleSettings(State.ToggleSettings);
	}

	if (State.bFrontCameraActive)
	{
		ToggleFrontCamera();
	}
}
```

注意：Toggle 恢复直接 `LAS->SetToggleSettings(State.ToggleSettings)` 整体赋值，后续新增开关零改动。

- [ ] **Step 3: 编译验证**

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected: `Result: Succeeded`

---

### Task 5: DebugPanel Toggle 写入时同步更新 Subsystem

**Files:**
- Modify: `Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

当前 `OnToggleCheckChanged` 写 `ULASettings` 但不写 `ULTDebugSubsystem`。Toggle 部分的 `DebugState` 在 Deinitialize 保存时不会反映最新修改。需要同步。

- [ ] **Step 1: 修改 OnToggleCheckChanged 同步写入 DebugState**

当前函数体：

```cpp
void SLTDebugPanel::OnToggleCheckChanged(FName PropertyName, ECheckBoxState NewState)
{
	ULASettings* Settings = ULASettings::Get();
	if (!Settings) return;

	FLAAnimToggleSettings S = Settings->GetToggleSettings();
	const bool bNew = (NewState == ECheckBoxState::Checked);

	if (PropertyName == TEXT("bUseBlendSpaceLoop")) S.bUseBlendSpaceLoop = bNew;
	else if (PropertyName == TEXT("bEnableStart")) S.bEnableStart = bNew;
	// ... 其余 else if ...
	else if (PropertyName == TEXT("bEnableLean")) S.bEnableLean = bNew;

	Settings->SetToggleSettings(S);
}
```

在 `Settings->SetToggleSettings(S);` 之后追加：

```cpp
if (DebugSubsystem.IsValid())
{
	FLTDebugState DS = DebugSubsystem->GetDebugState();
	DS.ToggleSettings = S;
	DebugSubsystem->SetDebugState(DS);
}
```

这里直接整体赋值 `DS.ToggleSettings = S`，无需逐字段 if-else。后续新增开关零改动。

- [ ] **Step 2: 在 SetCharacter 中捕获初始 CharacterSetIndex**

当前 `SetCharacter` 末尾已有 `DebugSubsystem->SetCMCParams(P);`。在其之后追加：

```cpp
DebugSubsystem->CaptureCurrentState(
	InCharacter->GetCurrentCharacterSetIndex(),
	InCharacter->bStrafeMovement,
	false
);
```

- [ ] **Step 3: 编译验证**

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected: `Result: Succeeded`

---

### Task 6: ToggleStrafe / ToggleFrontCamera / OnMouseWheel 同步 CaptureCurrentState

**Files:**
- Modify: `Source/LocomotionTemplate/Character/LTCharacter.cpp`

- [ ] **Step 1: 在 ToggleStrafe 末尾追加 CaptureCurrentState**

```cpp
void ALTCharacter::ToggleStrafe()
{
	bStrafeMovement = !bStrafeMovement;
	GetCharacterMovement()->bOrientRotationToMovement = !bStrafeMovement;
	bUseControllerRotationYaw = bStrafeMovement;

	if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
	{
		DS->CaptureCurrentState(CurrentCharacterSetIndex, bStrafeMovement, bFrontCameraActive);
	}
}
```

- [ ] **Step 2: 在 ToggleFrontCamera 末尾追加 CaptureCurrentState**

在 `ToggleFrontCamera` 函数体末尾（两个 if/else 分支之后）追加：

```cpp
if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
{
	DS->CaptureCurrentState(CurrentCharacterSetIndex, bStrafeMovement, bFrontCameraActive);
}
```

- [ ] **Step 3: 在 OnMouseWheel 末尾同步 MaxWalkSpeed 到 DebugState**

`OnMouseWheel` 直接改 `CMC->MaxWalkSpeed`，不走 `SetCMCParams`。DebugPanel 未打开时滚轮调速不会同步到 `DebugState`。在 `CMC->MaxWalkSpeed = NewSpeed;` 之后追加：

```cpp
if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
{
	FLTCMCParams P = DS->GetCMCParams();
	P.MaxWalkSpeed = NewSpeed;
	DS->SetCMCParams(P);
}
```

- [ ] **Step 4: 编译验证**

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected: `Result: Succeeded`

---

## 自检清单

### 1. Spec 覆盖

| 需求 | 对应 Task |
|------|-----------|
| CMC 参数持久化 | Task 2 (struct) + Task 3 (Subsystem) + Task 4 (BeginPlay恢复) |
| Toggle 持久化 | Task 2 + Task 3 + Task 4 + Task 5 |
| CharacterSet Index 持久化 | Task 2 + Task 3 + Task 4 + Task 5 |
| Strafe 模式持久化 | Task 2 + Task 4 + Task 6 |
| Front Camera 持久化 | Task 2 + Task 4 + Task 6 |
| 鼠标滚轮调速持久化 | Task 6 |
| 后续新增开关自动持久化 | `FLAAnimToggleSettings` 整体嵌套，新增字段自动包含 |

### 2. Placeholder 扫描

无 TBD / TODO / "implement later" 等。

### 3. 类型一致性

- `FLTDebugState` 在 Task 2 定义，内嵌 `FLAAnimToggleSettings`（来自 LyraAnim 插件）
- Task 3/4/5/6 所有 Toggle 操作均为整体赋值 `ToggleSettings = ...`，无逐字段 if-else
- `CaptureCurrentState(int32, bool, bool)` 签名在 Task 3 定义，Task 5/6 调用，参数类型匹配
- `GetDebugState()` 返回 `const FLTDebugState&`，Task 5 中做拷贝修改再 `SetDebugState`
