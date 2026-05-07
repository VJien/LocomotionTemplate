#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LTPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class LOCOMOTIONTEMPLATE_API ALTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALTPlayerController();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
