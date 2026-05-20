# Jump Z Debug UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add DebugPanel and top HUD support for editing and viewing `JumpZVelocity`.

**Architecture:** Extend existing CMC debug state with `JumpZVelocity`. Add a Project Settings range, restore the value in `ALTCharacter::BeginPlay`, expose it through a DebugPanel slider, and display it in the top HUD movement stats line.

**Tech Stack:** UE 5.7 C++, `UCharacterMovementComponent`, Slate, JSON debug state via `FJsonObjectConverter`.

---

## Task 1: Add State And Settings

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/Locomotion/LTLocomotionTypes.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Core/LTProjectSettings.h`
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Core/LTDebugSubsystem.cpp`

- [ ] **Step 1: Add debug state field**

In `FLTCMCParams`, add:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CMC")
float JumpZVelocity = 700.0f;
```

- [ ] **Step 2: Add Project Settings range**

In `ULTProjectSettings`, add under CMC ranges:

```cpp
UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "CMC")
FLTSliderRange JumpZVelocityRange;
```

- [ ] **Step 3: Initialize default state**

In `ULTDebugSubsystem::Initialize`, add:

```cpp
DefaultState.CMCParams.JumpZVelocity = PS->JumpZVelocityRange.Default;
```

Expected: reset/default state includes jump velocity.

## Task 2: Apply Runtime Value

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTCharacter.cpp`

- [ ] **Step 1: Restore JumpZVelocity on BeginPlay**

After braking deceleration restore, add:

```cpp
GetCharacterMovement()->JumpZVelocity = State.CMCParams.JumpZVelocity;
```

Expected: saved/debug default jump velocity is applied on Play.

## Task 3: Add DebugPanel Slider

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

- [ ] **Step 1: Add slider row after movement speed**

Add a `BuildSliderRow` slot using:

```cpp
BuildSliderRow(
	LOCTEXT("JumpZ", "跳跃力度"),
	PS ? PS->JumpZVelocityRange.Min : 200.f,
	PS ? PS->JumpZVelocityRange.Max : 1200.f,
	TAttribute<float>::CreateLambda([this]() -> float
	{
		if (!DebugSubsystem.IsValid()) return 0.f;
		return DebugSubsystem->GetCMCParams().JumpZVelocity;
	}),
	[this](float Val)
	{
		if (!DebugSubsystem.IsValid()) return;
		FLTCMCParams P = DebugSubsystem->GetCMCParams();
		P.JumpZVelocity = Val;
		DebugSubsystem->SetCMCParams(P);
		if (CachedCharacter.IsValid())
		{
			CachedCharacter->GetCharacterMovement()->JumpZVelocity = Val;
		}
	})
```

- [ ] **Step 2: Capture current value in `SetCharacter`**

Add:

```cpp
P.JumpZVelocity = CMC->JumpZVelocity;
```

Expected: opening DebugPanel reflects current character value.

## Task 4: Add Top HUD Display

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTHUD.cpp`

- [ ] **Step 1: Extend movement stats text**

Change format to:

```cpp
TEXT("Speed: %.0f  |  Accel: %.0f  |  Brake: %.0f  |  Jump: %.0f")
```

Add argument:

```cpp
Movement->JumpZVelocity
```

Expected: top HUD shows JumpZ beside existing movement values.

## Task 5: Verification

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

## Self-Review

- Spec coverage: `JumpZVelocity` state, ProjectSettings range, BeginPlay restore, DebugPanel slider, top HUD display.
- Placeholder scan: no placeholders.
- Type consistency: `JumpZVelocity` used consistently in `FLTCMCParams` and `UCharacterMovementComponent`.
