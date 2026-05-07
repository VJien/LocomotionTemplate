// Fill out your copyright notice in the Description page of Project Settings.


#include "LAAnimLinkComponent.h"

#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Engine/World.h"


// Sets default values for this component's properties
ULAAnimLinkComponent::ULAAnimLinkComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULAAnimLinkComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!bAutoLink)
	{
		return;
	}
	// 如果 MeshComponent 已经被外部初始化（如 InitPawnData），不要覆盖它
	if (!MeshComponent)
	{
		if (auto&& Character = Cast<ACharacter>(GetOwner()))
		{
			MeshComponent = Character->GetMesh();
		}
		else
		{
			MeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
		}
	}
	
	if (MeshComponent)
	{
		DefaultProfile.Link(MeshComponent);
	}
	
}


// Called every frame
void ULAAnimLinkComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bPendingLink && MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("LAAnimLinkComponent: Trying to link anim instance..."));
		if (MeshComponent->GetAnimInstance())
		{
			UE_LOG (LogTemp, Warning, TEXT("LAAnimLinkComponent: Anim instance ready, linking now. %s"),MeshComponent->GetOwner()? *MeshComponent->GetOwner()->GetName():TEXT("NoOwner"));
			DefaultProfile.Link(MeshComponent);
			bPendingLink = false;
		}
	}
}

void ULAAnimLinkComponent::InitializeAnimLayerConfig(USkeletalMeshComponent* InMeshComponent,
	const FLAAnimLinkProfile& InDefaultProfile, const TMap<FGameplayTag, FLAAnimLinkProfile>& InLinkProfiles)
{
	FString NetModeStr = IsRunningDedicatedServer() ? TEXT("DedicatedServer") : 
		(GetNetMode() == NM_ListenServer ? TEXT("ListenServer") : 
		(GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Standalone")));

	UE_LOG(LogTemp, Warning, TEXT("[Antigravity] Init Config for %s [%s]. InMesh=%s. ExistingMesh=%s"), 
		*GetNameSafe(GetOwner()), 
		*NetModeStr,
		InMeshComponent ? *InMeshComponent->GetName() : TEXT("NULL"),
		MeshComponent ? *MeshComponent->GetName() : TEXT("NULL"));

	if (!InMeshComponent)
	{
		return;
	}
	if (MeshComponent)
	{
		CurrentProfile.Unlink(MeshComponent);
		CurrentProfile.Reset();
		DefaultProfile.Unlink(MeshComponent);
	}

	MeshComponent = InMeshComponent;
	DefaultProfile = InDefaultProfile;
	LinkProfiles = InLinkProfiles;
	if (MeshComponent)
	{
		// 尝试立即链接
		if (HasBegunPlay() && MeshComponent->GetAnimInstance())
		{
			DefaultProfile.Link(MeshComponent);
			bPendingLink = false;
		}
		else
		{
			// 如果 AnimInstance 尚未准备好，或尚未 BeginPlay，标记为 Pending，在 Tick 中重试
			// 这能保证无论初始化时机如何，只要 AnimInstance 就绪就会执行链接
			bPendingLink = true;
			
			// 强制确保 Tick 启用，否则 TickComponent 可能不会运行
			if (!IsComponentTickEnabled())
			{
				SetComponentTickEnabled(true);
			}
			RegisterComponent(); 
		}
	}
}

bool ULAAnimLinkComponent::LinkAnimInstanceByTag(FGameplayTag ProfileTag)
{
	const FLAAnimLinkProfile* Profile = FindProfileByTag(ProfileTag);
	if (MeshComponent && Profile)
	{
		ResetAnimInstance();
		CurrentProfile = *Profile;
		CurrentProfile.Link(MeshComponent);
		return true;
	}
	return false;
}

void ULAAnimLinkComponent::ResetAnimInstance()
{
	if (MeshComponent)
	{
		CurrentProfile.Unlink(MeshComponent);
		DefaultProfile.Link(MeshComponent);
		CurrentProfile.Reset();
	}
}

const FLAAnimLinkProfile* ULAAnimLinkComponent::FindProfileByTag(FGameplayTag ProfileTag) const
{
	return  LinkProfiles.Find(ProfileTag);
}

