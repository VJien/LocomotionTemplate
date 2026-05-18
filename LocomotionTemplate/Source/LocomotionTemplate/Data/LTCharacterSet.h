#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LyraAnim/Data/LASettings.h"
#include "LTCharacterSet.generated.h"

class UAnimInstance;
class USkeletalMesh;
class UTexture2D;

UCLASS(BlueprintType)
class LOCOMOTIONTEMPLATE_API ULTCharacterSet : public UDataAsset
{
	GENERATED_BODY()

public:
	ULTCharacterSet();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	// Controls which animation toggles are shown in DebugPanel for this character set.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug UI")
	FLAAnimToggleSettings DebugToggleVisibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<TSubclassOf<UAnimInstance>> LinkedAnimClasses;
};
