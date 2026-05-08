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
