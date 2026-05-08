#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LTProjectSettings.generated.h"

class ULTCharacterSet;

USTRUCT(BlueprintType)
struct FLTSliderRange
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	float Min = 0.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	float Max = 1000.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	float Default = 500.0f;
};

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Locomotion Template"))
class LOCOMOTIONTEMPLATE_API ULTProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "CMC")
	FLTSliderRange MaxAccelerationRange;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "CMC")
	FLTSliderRange BrakingDecelerationRange;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "CMC")
	FLTSliderRange MaxWalkSpeedRange;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FLTSliderRange CameraArmLengthRange;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Character")
	TArray<TSoftObjectPtr<ULTCharacterSet>> CharacterSets;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "CMC", meta = (ClampMin = "1.0"))
	float SpeedScrollStep = 20.0f;

	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	static const ULTProjectSettings* Get();
};
