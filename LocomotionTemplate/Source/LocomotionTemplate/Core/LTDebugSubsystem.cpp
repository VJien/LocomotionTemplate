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
