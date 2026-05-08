#pragma once

#include "CoreMinimal.h"
#include "LyraAnim/Data/LASettings.h"
#include "LTLocomotionTypes.generated.h"

USTRUCT(BlueprintType)
struct FLTCMCParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CMC")
	float MaxAcceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CMC")
	float BrakingDeceleration = 512.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CMC")
	float MaxWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraArmLength = 400.0f;
};

USTRUCT(BlueprintType)
struct FLTDebugState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FLTCMCParams CMCParams;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FLAAnimToggleSettings ToggleSettings;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	int32 CharacterSetIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	bool bStrafeMovement = true;

	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	bool bFrontCameraActive = false;
};
