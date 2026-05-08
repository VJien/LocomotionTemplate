#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "LTCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimInstance;
class ULTCharacterSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLTCharacterSetChangedSignature, int32, NewIndex);

UCLASS()
class LOCOMOTIONTEMPLATE_API ALTCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALTCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Character Set")
	bool ApplyCharacterSet(int32 Index);

	UFUNCTION(BlueprintPure, Category = "Character Set")
	int32 GetCurrentCharacterSetIndex() const { return CurrentCharacterSetIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Character Set")
	FLTCharacterSetChangedSignature OnCharacterSetChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FrontCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleCameraAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleStrafeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DebugPanelAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Front")
	float FrontCameraDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Front")
	float FrontCameraHeight = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bStrafeMovement = true;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character Set", meta = (AllowPrivateAccess = "true"))
	int32 CurrentCharacterSetIndex = INDEX_NONE;

	TArray<TSubclassOf<UAnimInstance>> ActiveLinkedAnimClasses;

	bool bFrontCameraActive = false;
	FVector FrontCameraWorldOffset;

	bool ApplyCharacterSetAsset(const ULTCharacterSet* CharacterSet, int32 Index);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleFrontCamera();
	void ToggleStrafe();
	void JumpStarted();
	void JumpCompleted();
	void ToggleDebugPanel();
	void OnMouseWheel(float AxisValue);

	void UpdateFrontCamera();
};
