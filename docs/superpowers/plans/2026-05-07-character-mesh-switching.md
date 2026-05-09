# 角色Mesh切换系统 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现Project Settings配置角色DA数组，并在DebugPanel中用按钮运行时切换角色SkeletalMesh、ABP和LinkedAnim layers。

**Architecture:** 新增`ULTCharacterSet` DataAsset承载单个角色配置。`ULTProjectSettings`保存角色配置数组。`ALTCharacter`负责应用配置，`SLTDebugPanel`根据配置数组动态生成按钮并调用角色切换接口。

**Tech Stack:** UE 5.7 C++、UDataAsset、UDeveloperSettings、ACharacter、USkeletalMeshComponent、Slate DebugPanel。

---

### Task 1: 新增角色配置DataAsset

**Files:**
- Create: `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.h`
- Create: `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.cpp`

- [ ] **Step 1: 添加DataAsset头文件**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LTCharacterSet.generated.h"

class UAnimInstance;
class USkeletalMesh;
class UTexture2D;

UCLASS(BlueprintType)
class LOCOMOTIONTEMPLATE_API ULTCharacterSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<TSubclassOf<UAnimInstance>> LinkedAnimClasses;
};
```

- [ ] **Step 2: 添加空cpp**

```cpp
#include "Data/LTCharacterSet.h"
```

### Task 2: Project Settings配置数组

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Core/LTProjectSettings.h`

- [ ] **Step 1: 前向声明并添加CharacterSets数组**

```cpp
class ULTCharacterSet;

UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Character")
TArray<TSoftObjectPtr<ULTCharacterSet>> CharacterSets;
```

### Task 3: ALTCharacter切换接口

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTCharacter.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTCharacter.cpp`

- [ ] **Step 1: 头文件新增接口和状态**

```cpp
class ULTCharacterSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLTCharacterSetChangedSignature, int32, NewIndex);

UFUNCTION(BlueprintCallable, Category = "Character Set")
bool ApplyCharacterSet(int32 Index);

UFUNCTION(BlueprintPure, Category = "Character Set")
int32 GetCurrentCharacterSetIndex() const { return CurrentCharacterSetIndex; }

UPROPERTY(BlueprintAssignable, Category = "Character Set")
FLTCharacterSetChangedSignature OnCharacterSetChanged;

UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character Set", meta = (AllowPrivateAccess = "true"))
int32 CurrentCharacterSetIndex = INDEX_NONE;

TArray<TSubclassOf<UAnimInstance>> ActiveLinkedAnimClasses;

bool ApplyCharacterSetAsset(const ULTCharacterSet* CharacterSet, int32 Index);
```

- [ ] **Step 2: cpp实现配置读取和应用**

```cpp
bool ALTCharacter::ApplyCharacterSet(int32 Index)
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	if (!Settings || !Settings->CharacterSets.IsValidIndex(Index))
	{
		return false;
	}

	const ULTCharacterSet* CharacterSet = Settings->CharacterSets[Index].LoadSynchronous();
	return ApplyCharacterSetAsset(CharacterSet, Index);
}

bool ALTCharacter::ApplyCharacterSetAsset(const ULTCharacterSet* CharacterSet, int32 Index)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !CharacterSet || !CharacterSet->SkeletalMesh || !CharacterSet->AnimClass)
	{
		return false;
	}

	for (const TSubclassOf<UAnimInstance>& LayerClass : ActiveLinkedAnimClasses)
	{
		if (LayerClass)
		{
			MeshComponent->UnlinkAnimClassLayers(LayerClass);
		}
	}
	ActiveLinkedAnimClasses.Reset();

	MeshComponent->SetSkeletalMesh(CharacterSet->SkeletalMesh);
	MeshComponent->SetAnimInstanceClass(CharacterSet->AnimClass);

	for (const TSubclassOf<UAnimInstance>& LayerClass : CharacterSet->LinkedAnimClasses)
	{
		if (LayerClass)
		{
			MeshComponent->LinkAnimClassLayers(LayerClass);
			ActiveLinkedAnimClasses.Add(LayerClass);
		}
	}

	CurrentCharacterSetIndex = Index;
	OnCharacterSetChanged.Broadcast(CurrentCharacterSetIndex);
	return true;
}
```

### Task 4: DebugPanel按钮区

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

- [ ] **Step 1: 头文件新增构建方法**

```cpp
TSharedRef<SWidget> BuildCharacterSetSection();
FReply OnCharacterSetClicked(int32 Index);
FSlateColor GetCharacterSetButtonColor(int32 Index) const;
FText GetCharacterSetDisplayName(int32 Index) const;
```

- [ ] **Step 2: 在Construct中Reset按钮前插入Section**

```cpp
+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
[
	BuildCharacterSetSection()
]
```

- [ ] **Step 3: 实现Section和点击回调**

```cpp
TSharedRef<SWidget> SLTDebugPanel::BuildCharacterSetSection()
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

	Box->AddSlot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CharacterSetHeader", "Character Sets"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
	];

	if (!Settings || Settings->CharacterSets.Num() == 0)
	{
		Box->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoCharacterSets", "No character sets configured"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor::Gray)
		];
		return Box;
	}

	for (int32 Index = 0; Index < Settings->CharacterSets.Num(); ++Index)
	{
		Box->AddSlot().AutoHeight().Padding(0, 1)
		[
			SNew(SButton)
			.Text(this, &SLTDebugPanel::GetCharacterSetDisplayName, Index)
			.ButtonColorAndOpacity(this, &SLTDebugPanel::GetCharacterSetButtonColor, Index)
			.OnClicked(this, &SLTDebugPanel::OnCharacterSetClicked, Index)
		];
	}

	return Box;
}
```

### Task 5: 验证

**Files:**
- Verify: C++编译

- [ ] **Step 1: 运行git diff检查范围**

Run: `git diff -- LocomotionTemplate/Source/LocomotionTemplate docs/superpowers/plans/2026-05-07-character-mesh-switching.md`

- [ ] **Step 2: 用IDE或引擎BuildRecipe编译项目**

Expected: `LocomotionTemplate`模块编译通过。
