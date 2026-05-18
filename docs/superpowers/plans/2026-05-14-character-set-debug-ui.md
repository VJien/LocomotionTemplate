# CharacterSet DebugUI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add per-CharacterSet DebugPanel toggle visibility and description text.

**Architecture:** `ULTCharacterSet` stores `Description` plus `DebugToggleVisibility` using existing `FLAAnimToggleSettings`. `SLTDebugPanel` queries the current CharacterSet dynamically and collapses rows whose visibility flag is false while preserving current animation toggle state logic.

**Tech Stack:** UE 5.7 C++, Slate, `UDataAsset`, `FLAAnimToggleSettings`, Build.bat verification.

---

## File Structure

- Modify `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.h`: add `Description`, `DebugToggleVisibility`, constructor declaration, and `LASettings.h` include.
- Modify `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.cpp`: initialize `DebugToggleVisibility.bUseBlendSpaceLoop = true`.
- Modify `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.h`: add helpers for current CharacterSet, description, description visibility, and toggle visibility.
- Modify `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`: bind Toggle row visibility and add description section.

## Task 1: CharacterSet Data Fields

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.cpp`

- [ ] **Step 1: Add LASettings include and constructor/property declarations**

In `LTCharacterSet.h`, add:

```cpp
#include "LyraAnim/Data/LASettings.h"
```

Add public constructor and fields:

```cpp
ULTCharacterSet();

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
FText Description;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug UI")
FLAAnimToggleSettings DebugToggleVisibility;
```

- [ ] **Step 2: Initialize BlendSpace visibility default**

In `LTCharacterSet.cpp`, implement:

```cpp
ULTCharacterSet::ULTCharacterSet()
{
	DebugToggleVisibility.bUseBlendSpaceLoop = true;
}
```

Expected: default `DebugToggleVisibility` effectively shows all existing DebugPanel animation toggle rows.

## Task 2: DebugPanel Current Set Helpers

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

- [ ] **Step 1: Declare helper functions**

Add private declarations:

```cpp
const ULTCharacterSet* GetCurrentCharacterSet() const;
FText GetCurrentCharacterSetDescription() const;
EVisibility GetCharacterSetDescriptionVisibility() const;
bool IsToggleVisibleForCurrentCharacterSet(FName PropertyName) const;
```

Forward declare `ULTCharacterSet` in the header.

- [ ] **Step 2: Implement current CharacterSet lookup**

Add implementation:

```cpp
const ULTCharacterSet* SLTDebugPanel::GetCurrentCharacterSet() const
{
	if (!CachedCharacter.IsValid()) return nullptr;

	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	if (!Settings) return nullptr;

	const int32 Index = CachedCharacter->GetCurrentCharacterSetIndex();
	if (!Settings->CharacterSets.IsValidIndex(Index)) return nullptr;

	return Settings->CharacterSets[Index].LoadSynchronous();
}
```

- [ ] **Step 3: Implement description helpers**

Add implementation:

```cpp
FText SLTDebugPanel::GetCurrentCharacterSetDescription() const
{
	const ULTCharacterSet* CharacterSet = GetCurrentCharacterSet();
	return CharacterSet ? CharacterSet->Description : FText::GetEmpty();
}

EVisibility SLTDebugPanel::GetCharacterSetDescriptionVisibility() const
{
	return GetCurrentCharacterSetDescription().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}
```

- [ ] **Step 4: Implement toggle visibility mapping**

Add implementation mirroring existing toggle property names:

```cpp
bool SLTDebugPanel::IsToggleVisibleForCurrentCharacterSet(FName PropertyName) const
{
	const ULTCharacterSet* CharacterSet = GetCurrentCharacterSet();
	if (!CharacterSet) return true;

	const FLAAnimToggleSettings& S = CharacterSet->DebugToggleVisibility;
	if (PropertyName == TEXT("bUseBlendSpaceLoop")) return S.bUseBlendSpaceLoop;
	else if (PropertyName == TEXT("bEnableStart")) return S.bEnableStart;
	else if (PropertyName == TEXT("bEnableStartDM")) return S.bEnableStartDM;
	else if (PropertyName == TEXT("bEnableStartStrideWarping")) return S.bEnableStartStrideWarping;
	else if (PropertyName == TEXT("bEnableStop")) return S.bEnableStop;
	else if (PropertyName == TEXT("bEnableStopDM")) return S.bEnableStopDM;
	else if (PropertyName == TEXT("bEnableStopStrideWarping")) return S.bEnableStopStrideWarping;
	else if (PropertyName == TEXT("bEnableCycleDM")) return S.bEnableCycleDM;
	else if (PropertyName == TEXT("bEnableCycleStrideWarping")) return S.bEnableCycleStrideWarping;
	else if (PropertyName == TEXT("bEnablePivot")) return S.bEnablePivot;
	else if (PropertyName == TEXT("bEnablePivotStrideWarping")) return S.bEnablePivotStrideWarping;
	else if (PropertyName == TEXT("bEnableAimOffset")) return S.bEnableAimOffset;
	else if (PropertyName == TEXT("bEnableTurnInPlace")) return S.bEnableTurnInPlace;
	else if (PropertyName == TEXT("bEnableFootIK")) return S.bEnableFootIK;
	else if (PropertyName == TEXT("bEnableLean")) return S.bEnableLean;

	return true;
}
```

## Task 3: DebugPanel Visibility And Description UI

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

- [ ] **Step 1: Bind non-conditional toggle row visibility**

Wrap `BuildToggleRow` return root with:

```cpp
.Visibility_Lambda([this, PropertyName]() -> EVisibility
{
	return IsToggleVisibleForCurrentCharacterSet(PropertyName) ? EVisibility::Visible : EVisibility::Collapsed;
})
```

Expected: rows built by `BuildToggleRow` hide when current CharacterSet visibility is false.

- [ ] **Step 2: Bind conditional toggle row visibility**

Update `BuildConditionalToggleRow` visibility lambda:

```cpp
RowSlot->SetVisibility(TAttribute<EVisibility>::CreateLambda([this, PropertyName, IsEnabledAttr]() -> EVisibility
{
	if (!IsToggleVisibleForCurrentCharacterSet(PropertyName)) return EVisibility::Collapsed;
	if (!IsEnabledAttr.Get()) return EVisibility::Collapsed;
	return EVisibility::Visible;
}));
```

Expected: conditional rows require both CharacterSet visibility and existing dependency logic.

- [ ] **Step 3: Add description section after Character Sets**

After `BuildCharacterSetSection()` slot, add:

```cpp
+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
[
	SNew(SBorder)
	.Visibility(this, &SLTDebugPanel::GetCharacterSetDescriptionVisibility)
	.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f))
	.Padding(6.0f)
	[
		SNew(STextBlock)
		.Text(this, &SLTDebugPanel::GetCurrentCharacterSetDescription)
		.AutoWrapText(true)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f))
	]
]
```

Expected: description appears only for current CharacterSet with non-empty `Description`.

## Task 4: Verification

**Files:**
- Build only.

- [ ] **Step 1: Compile editor target**

Run:

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

Expected output contains:

```text
Result: Succeeded
```

- [ ] **Step 2: Inspect diff**

Run:

```powershell
git diff -- LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.h LocomotionTemplate/Source/LocomotionTemplate/Data/LTCharacterSet.cpp LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.h LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp docs/superpowers/specs/2026-05-14-character-set-debug-ui-design.md docs/superpowers/plans/2026-05-14-character-set-debug-ui.md
```

Expected: only planned files changed; no `Content/` assets changed.

## Self-Review

- Spec coverage: data fields, description UI, per-toggle visibility, no plugin behavior changes, verification covered.
- Placeholder scan: no placeholders.
- Type consistency: `DebugToggleVisibility`, `Description`, and helper names match across tasks.
