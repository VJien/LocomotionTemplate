#include "Character/LTCharacter.h"
#include "Character/LTHUD.h"
#include "Core/LTProjectSettings.h"
#include "Core/LTDebugSubsystem.h"
#include "Data/LTCharacterSet.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "LyraAnim/Data/LASettings.h"

ALTCharacter::ALTCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	bStrafeMovement = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	bUseControllerRotationYaw = true;

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

void ALTCharacter::BeginPlay()
{
	Super::BeginPlay();

	ULTDebugSubsystem* DebugSS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>();
	const FLTDebugState& State = DebugSS->GetDebugState();

	GetCharacterMovement()->MaxAcceleration = State.CMCParams.MaxAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = State.CMCParams.BrakingDeceleration;
	GetCharacterMovement()->MaxWalkSpeed = State.CMCParams.MaxWalkSpeed;
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = State.CMCParams.CameraArmLength;
	}

	bStrafeMovement = State.bStrafeMovement;
	GetCharacterMovement()->bOrientRotationToMovement = !bStrafeMovement;
	bUseControllerRotationYaw = bStrafeMovement;

	ApplyCharacterSet(State.CharacterSetIndex);

	if (ULASettings* LAS = ULASettings::Get())
	{
		LAS->SetToggleSettings(State.ToggleSettings);
	}

	if (State.bFrontCameraActive)
	{
		ToggleFrontCamera();
	}
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
	if (DebugPanelAction)
	{
		EnhancedInput->BindAction(DebugPanelAction, ETriggerEvent::Started, this, &ALTCharacter::ToggleDebugPanel);
	}
	if (ScrollAction)
	{
		EnhancedInput->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &ALTCharacter::OnMouseWheel);
	}
}

bool ALTCharacter::ApplyCharacterSet(int32 Index)
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	if (!Settings || !Settings->CharacterSets.IsValidIndex(Index))
	{
		return false;
	}

	const ULTCharacterSet* CharacterSet = Settings->CharacterSets[Index].LoadSynchronous();
	return ApplyCharacterSetAsset(CharacterSet, Index);
}

bool ALTCharacter::ApplyCharacterSetAsset(const ULTCharacterSet* CharacterSet, int32 Index)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !CharacterSet || !CharacterSet->SkeletalMesh || !CharacterSet->AnimClass)
	{
		return false;
	}

	for (const TSubclassOf<UAnimInstance>& LayerClass : ActiveLinkedAnimClasses)
	{
		if (LayerClass)
		{
			MeshComponent->UnlinkAnimClassLayers(LayerClass);
		}
	}
	ActiveLinkedAnimClasses.Reset();

	MeshComponent->SetSkeletalMesh(CharacterSet->SkeletalMesh);
	MeshComponent->SetAnimInstanceClass(CharacterSet->AnimClass);

	for (const TSubclassOf<UAnimInstance>& LayerClass : CharacterSet->LinkedAnimClasses)
	{
		if (LayerClass)
		{
			MeshComponent->LinkAnimClassLayers(LayerClass);
			ActiveLinkedAnimClasses.Add(LayerClass);
		}
	}

	CurrentCharacterSetIndex = Index;
	OnCharacterSetChanged.Broadcast(CurrentCharacterSetIndex);
	return true;
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

	if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
	{
		DS->CaptureCurrentState(CurrentCharacterSetIndex, bStrafeMovement, bFrontCameraActive);
	}
}

void ALTCharacter::ToggleStrafe()
{
	bStrafeMovement = !bStrafeMovement;
	GetCharacterMovement()->bOrientRotationToMovement = !bStrafeMovement;
	bUseControllerRotationYaw = bStrafeMovement;

	if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
	{
		DS->CaptureCurrentState(CurrentCharacterSetIndex, bStrafeMovement, bFrontCameraActive);
	}
}

void ALTCharacter::JumpStarted()
{
	Jump();
}

void ALTCharacter::JumpCompleted()
{
	StopJumping();
}

void ALTCharacter::ToggleDebugPanel()
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (ALTHUD* HUD = Cast<ALTHUD>(PC->GetHUD()))
		{
			HUD->ToggleDebugPanel();
		}
	}
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

void ALTCharacter::OnMouseWheel(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();
	if (FMath::Abs(AxisValue) < KINDA_SMALL_NUMBER) return;

	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	if (!Settings) return;

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;

	const APlayerController* PC = Cast<APlayerController>(Controller);
	const bool bAltDown = PC && (PC->IsInputKeyDown(EKeys::LeftAlt) || PC->IsInputKeyDown(EKeys::RightAlt));
	const bool bCtrlDown = PC && (PC->IsInputKeyDown(EKeys::LeftControl) || PC->IsInputKeyDown(EKeys::RightControl));

	if (ULTDebugSubsystem* DS = GetGameInstance()->GetSubsystem<ULTDebugSubsystem>())
	{
		FLTCMCParams P = DS->GetCMCParams();
		const float Step = Settings->SpeedScrollStep;

		if (bAltDown)
		{
			const float NewValue = FMath::Clamp(CMC->MaxAcceleration + AxisValue * Step, Settings->MaxAccelerationRange.Min, Settings->MaxAccelerationRange.Max);
			CMC->MaxAcceleration = NewValue;
			P.MaxAcceleration = NewValue;
		}
		else if (bCtrlDown)
		{
			const float NewValue = FMath::Clamp(CMC->BrakingDecelerationWalking + AxisValue * Step, Settings->BrakingDecelerationRange.Min, Settings->BrakingDecelerationRange.Max);
			CMC->BrakingDecelerationWalking = NewValue;
			P.BrakingDeceleration = NewValue;
		}
		else
		{
			const float NewValue = FMath::Clamp(CMC->MaxWalkSpeed + AxisValue * Step, Settings->MaxWalkSpeedRange.Min, Settings->MaxWalkSpeedRange.Max);
			CMC->MaxWalkSpeed = NewValue;
			P.MaxWalkSpeed = NewValue;
		}

		DS->SetCMCParams(P);
	}
}
