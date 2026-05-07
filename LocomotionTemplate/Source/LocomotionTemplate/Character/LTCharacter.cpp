#include "Character/LTCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

ALTCharacter::ALTCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrontCamera"));
	FrontCamera->SetupAttachment(RootComponent);
	FrontCamera->bAutoActivate = false;
}

void ALTCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFrontCameraActive)
	{
		UpdateFrontCamera();
	}
}

void ALTCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALTCharacter::Move);
	}
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALTCharacter::Look);
	}
	if (ToggleCameraAction)
	{
		EnhancedInput->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ALTCharacter::ToggleFrontCamera);
	}
	if (ToggleStrafeAction)
	{
		EnhancedInput->BindAction(ToggleStrafeAction, ETriggerEvent::Started, this, &ALTCharacter::ToggleStrafe);
	}
	if (JumpAction)
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ALTCharacter::JumpStarted);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ALTCharacter::JumpCompleted);
	}
}

void ALTCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVector.Y);
	AddMovementInput(RightDirection, MoveVector.X);
}

void ALTCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void ALTCharacter::ToggleFrontCamera()
{
	bFrontCameraActive = !bFrontCameraActive;

	if (bFrontCameraActive)
	{
		const FRotator YawRot(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		FrontCameraWorldOffset = YawRot.RotateVector(FVector(FrontCameraDistance, 0.0f, FrontCameraHeight));

		FollowCamera->Deactivate();
		FrontCamera->Activate();

		UpdateFrontCamera();
	}
	else
	{
		FrontCamera->Deactivate();
		FollowCamera->Activate();
	}
}

void ALTCharacter::ToggleStrafe()
{
	bStrafeMovement = !bStrafeMovement;
	GetCharacterMovement()->bOrientRotationToMovement = !bStrafeMovement;
	bUseControllerRotationYaw = bStrafeMovement;
}

void ALTCharacter::JumpStarted()
{
	Jump();
}

void ALTCharacter::JumpCompleted()
{
	StopJumping();
}

void ALTCharacter::UpdateFrontCamera()
{
	const FVector TargetPos = GetActorLocation() + FrontCameraWorldOffset;
	FrontCamera->SetWorldLocation(TargetPos);

	const FVector ToActor = GetActorLocation() - TargetPos;
	FRotator LookRot = ToActor.Rotation();
	LookRot.Pitch = FMath::Clamp(LookRot.Pitch, -30.0f, 30.0f);
	FrontCamera->SetWorldRotation(LookRot);
}
