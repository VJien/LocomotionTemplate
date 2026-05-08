// Fill out your copyright notice in the Description page of Project Settings.


#include "LAAnimInstance.h"


#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LAAnimInstance)


ULAAnimInstance::ULAAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULAAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}

void ULAAnimInstance::ApplyAdditiveHurt_Implementation(ELADirection4 Direction)
{
	HurtDirection = Direction;
	if(bHurtA)
	{
		bHurtA = false;
		bHurtB = true;
	}
	else
	{
		bHurtA = true;
		bHurtB = false;
	}
}

void ULAAnimInstance::StopAdditiveHurt_Implementation()
{
	bHurtA = false;
	bHurtB = false;
}

void ULAAnimInstance::SetGait(ELAGait InTargetGait)
{
	DesireGait = InTargetGait;
}

FLAAnimToggleSettings ULAAnimInstance::GetAnimToggleSettings() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings() : FLAAnimToggleSettings();
}

#if WITH_EDITOR
EDataValidationResult ULAAnimInstance::IsDataValid(FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	GameplayTagPropertyMap.IsDataValid(this, Context);

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}

#endif // WITH_EDITOR



const FLACharacterGroundInfo& ULAAnimInstance::GetGroundInfo()
{
	auto&& CharacterOwner = Cast<ACharacter>(GetOwningActor());
	
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}
	auto&& CMC = CharacterOwner->GetCharacterMovement();
	if (!CMC)
	{
		return CachedGroundInfo;
	}
	if (CMC->MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CMC->CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (CharacterOwner->GetCapsuleComponent() ? CharacterOwner->GetCapsuleComponent()->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(CharacterOwner->GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		CharacterOwner->GetCapsuleComponent()->InitSweepCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = GroundTraceDistance;

		if (CMC->MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}
void ULAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void ULAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const ACharacter* Character = Cast<ACharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}
	const FLACharacterGroundInfo& GroundInfo = GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
	AnimToggleSettings = GetAnimToggleSettings();
}

