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

	UFUNCTION(BlueprintPure, Category = "Locomotion|Debug")
	const FLTToggleSettings& GetToggleSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Debug")
	void SetToggleSettings(const FLTToggleSettings& InSettings);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Debug")
	void ResetToDefaults();

	UPROPERTY(BlueprintAssignable, Category = "Locomotion|Debug")
	FLTToggleChanged OnToggleChanged;

private:
	FLTToggleSettings Settings;
	FLTToggleSettings Defaults;
};
