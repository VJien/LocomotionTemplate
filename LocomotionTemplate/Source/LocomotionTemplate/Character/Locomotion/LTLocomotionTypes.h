#pragma once

#include "CoreMinimal.h"
#include "LTLocomotionTypes.generated.h"

USTRUCT(BlueprintType)
struct FLTToggleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bUseBlendSpaceLoop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bUseOrientationWarping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableStop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableAimOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableTurnInPlace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnablePivot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableJump = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableLean = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableFootIK = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bEnableMotionMatching = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLTToggleChanged, FName, PropertyName, bool, bNewValue);
