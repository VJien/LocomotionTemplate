#include "Core/LTDebugSubsystem.h"

void ULTDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Defaults = FLTToggleSettings();
	Settings = Defaults;
}

const FLTToggleSettings& ULTDebugSubsystem::GetToggleSettings() const
{
	return Settings;
}

void ULTDebugSubsystem::SetToggleSettings(const FLTToggleSettings& InSettings)
{
	if (Settings.bUseBlendSpaceLoop != InSettings.bUseBlendSpaceLoop)
	{
		Settings.bUseBlendSpaceLoop = InSettings.bUseBlendSpaceLoop;
		OnToggleChanged.Broadcast(FName(TEXT("bUseBlendSpaceLoop")), Settings.bUseBlendSpaceLoop);
	}
	if (Settings.bUseOrientationWarping != InSettings.bUseOrientationWarping)
	{
		Settings.bUseOrientationWarping = InSettings.bUseOrientationWarping;
		OnToggleChanged.Broadcast(FName(TEXT("bUseOrientationWarping")), Settings.bUseOrientationWarping);
	}
	if (Settings.bEnableStart != InSettings.bEnableStart)
	{
		Settings.bEnableStart = InSettings.bEnableStart;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableStart")), Settings.bEnableStart);
	}
	if (Settings.bEnableStop != InSettings.bEnableStop)
	{
		Settings.bEnableStop = InSettings.bEnableStop;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableStop")), Settings.bEnableStop);
	}
	if (Settings.bEnableAimOffset != InSettings.bEnableAimOffset)
	{
		Settings.bEnableAimOffset = InSettings.bEnableAimOffset;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableAimOffset")), Settings.bEnableAimOffset);
	}
	if (Settings.bEnableTurnInPlace != InSettings.bEnableTurnInPlace)
	{
		Settings.bEnableTurnInPlace = InSettings.bEnableTurnInPlace;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableTurnInPlace")), Settings.bEnableTurnInPlace);
	}
	if (Settings.bEnablePivot != InSettings.bEnablePivot)
	{
		Settings.bEnablePivot = InSettings.bEnablePivot;
		OnToggleChanged.Broadcast(FName(TEXT("bEnablePivot")), Settings.bEnablePivot);
	}
	if (Settings.bEnableJump != InSettings.bEnableJump)
	{
		Settings.bEnableJump = InSettings.bEnableJump;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableJump")), Settings.bEnableJump);
	}
	if (Settings.bEnableLean != InSettings.bEnableLean)
	{
		Settings.bEnableLean = InSettings.bEnableLean;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableLean")), Settings.bEnableLean);
	}
	if (Settings.bEnableFootIK != InSettings.bEnableFootIK)
	{
		Settings.bEnableFootIK = InSettings.bEnableFootIK;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableFootIK")), Settings.bEnableFootIK);
	}
	if (Settings.bEnableMotionMatching != InSettings.bEnableMotionMatching)
	{
		Settings.bEnableMotionMatching = InSettings.bEnableMotionMatching;
		OnToggleChanged.Broadcast(FName(TEXT("bEnableMotionMatching")), Settings.bEnableMotionMatching);
	}
}

void ULTDebugSubsystem::ResetToDefaults()
{
	SetToggleSettings(Defaults);
}
