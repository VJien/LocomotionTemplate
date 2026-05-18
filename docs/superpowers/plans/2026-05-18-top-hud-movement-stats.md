# Top HUD Movement Stats Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Show walk speed, acceleration, and braking deceleration in the top HUD hint bar.

**Architecture:** Reuse existing `ALTHUD::ShowHintBar()` Slate text lambda. Read values directly from current pawn `UCharacterMovementComponent` so DebugPanel slider changes are reflected automatically.

**Tech Stack:** UE 5.7 C++, Slate, `UCharacterMovementComponent`.

---

## Task 1: Update Top HUD Text

**Files:**
- Modify: `LocomotionTemplate/Source/LocomotionTemplate/Character/LTHUD.cpp`

- [ ] **Step 1: Replace current speed-only text**

Change the existing lambda from:

```cpp
const float Speed = Character->GetCharacterMovement()->MaxWalkSpeed;
return FText::FromString(FString::Printf(TEXT("Speed: %.0f"), Speed));
```

To:

```cpp
const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
return FText::FromString(FString::Printf(
	TEXT("Speed: %.0f  |  Accel: %.0f  |  Brake: %.0f"),
	Movement->MaxWalkSpeed,
	Movement->MaxAcceleration,
	Movement->BrakingDecelerationWalking));
```

Expected: top HUD shows speed, acceleration, and braking deceleration on one line.

## Task 2: Verification

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

- Spec coverage: user asked top debug info to also show speed, acceleration, brake speed; plan does that.
- Placeholder scan: no placeholders.
- Type consistency: uses existing `UCharacterMovementComponent` fields already included in `LTHUD.cpp`.
