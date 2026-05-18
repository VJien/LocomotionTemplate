# Scroll CMC Tuning Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend mouse wheel CMC tuning to acceleration and braking deceleration, and quantize DebugPanel sliders to step 5.

**Architecture:** Keep the existing `MouseWheelAxis` binding. In `ALTCharacter::OnMouseWheel`, choose target CMC field by keyboard modifier, use `ULTProjectSettings::SpeedScrollStep` as shared delta, clamp with each field's range, and sync `ULTDebugSubsystem`. In `SLTDebugPanel::BuildSliderRow`, quantize denormalized slider values to increments of 5 before applying.

**Tech Stack:** UE 5.7 C++, legacy axis binding, Slate `SSlider`, `UCharacterMovementComponent`.

---

## Task 1: Modifier Mouse Wheel Tuning

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTCharacter.cpp`

- [ ] **Step 1: Include key definitions**

Add:

```cpp
#include "InputCoreTypes.h"
```

- [ ] **Step 2: Update `OnMouseWheel` target selection**

Replace speed-only logic with modifier branching:

```cpp
const APlayerController* PC = Cast<APlayerController>(Controller);
const bool bAltDown = PC && (PC->IsInputKeyDown(EKeys::LeftAlt) || PC->IsInputKeyDown(EKeys::RightAlt));
const bool bCtrlDown = PC && (PC->IsInputKeyDown(EKeys::LeftControl) || PC->IsInputKeyDown(EKeys::RightControl));

const float Step = Settings->SpeedScrollStep;

if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
{
	FLTCMCParams P = DS->GetCMCParams();
	if (bAltDown)
	{
		const float NewValue = FMath::Clamp(CMC->MaxAcceleration + AxisValue * Step, Settings->MaxAccelerationRange.Min, Settings->MaxAccelerationRange.Max);
		CMC->MaxAcceleration = NewValue;
		P.MaxAcceleration = NewValue;
	}
	else if (bCtrlDown)
	{
		const float NewValue = FMath::Clamp(CMC->BrakingDecelerationWalking + AxisValue * Step, Settings->BrakingDecelerationRange.Min, Settings->BrakingDecelerationRange.Max);
		CMC->BrakingDecelerationWalking = NewValue;
		P.BrakingDeceleration = NewValue;
	}
	else
	{
		const float NewValue = FMath::Clamp(CMC->MaxWalkSpeed + AxisValue * Step, Settings->MaxWalkSpeedRange.Min, Settings->MaxWalkSpeedRange.Max);
		CMC->MaxWalkSpeed = NewValue;
		P.MaxWalkSpeed = NewValue;
	}
	DS->SetCMCParams(P);
}
```

Expected behavior: Alt has priority over Ctrl. Normal wheel keeps existing speed behavior.

## Task 2: Slider Step Quantization

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/UI/SLTDebugPanel.cpp`

- [ ] **Step 1: Quantize denormalized slider value**

In `BuildSliderRow()` `OnValueChanged_Lambda`, change:

```cpp
float Denorm = MinValue + Normalized * (MaxValue - MinValue);
OnChanged(Denorm);
```

To:

```cpp
float Denorm = MinValue + Normalized * (MaxValue - MinValue);
Denorm = FMath::RoundToFloat(Denorm / 5.0f) * 5.0f;
Denorm = FMath::Clamp(Denorm, MinValue, MaxValue);
OnChanged(Denorm);
```

Expected behavior: all DebugPanel sliders apply values in increments of 5.

## Task 3: Verification

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

- Spec coverage: normal wheel speed, Alt wheel acceleration, Ctrl wheel braking, shared `SpeedScrollStep`, slider step 5.
- Placeholder scan: no placeholders.
- Type consistency: field names match `FLTCMCParams`, `UCharacterMovementComponent`, and `ULTProjectSettings`.
