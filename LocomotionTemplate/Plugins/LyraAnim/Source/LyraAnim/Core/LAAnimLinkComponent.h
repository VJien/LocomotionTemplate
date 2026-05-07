// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "LyraAnim/Data/LATypes.h"
#include "LAAnimLinkComponent.generated.h"

/**
 * 动画层链接组件
 *
 * 管理 SkeletalMeshComponent 上的动画层动态切换。
 * 支持:
 * - 默认动画层配置 (DefaultProfile)
 * - 按 GameplayTag 切换动画层配置 (LinkProfiles)
 * - 自动/手动初始化
 *
 * 替代原版 Lyra 中通过 B_WeaponInstance_Base 在武器切换时
 * 调用 LinkAnimClassLayers 的模式，将其封装为可复用的 ActorComponent。
 *
 * 使用流程:
 * 1. 将此组件添加到 Character
 * 2. 配置 DefaultProfile (默认动画层)
 * 3. 配置 LinkProfiles (按 Tag 映射不同动画层，如武器类型)
 * 4. 在武器切换时调用 LinkAnimInstanceByTag() 切换
 * 5. 或设置 bAutoLink=true 让组件在 BeginPlay 自动链接
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAANIM_API ULAAnimLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULAAnimLinkComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 初始化动画层配置 (由外部调用)
	 * Initialize the component with a specific mesh component and profile configuration.
	 * Handles unlinking previous profiles, setting up the new mesh, and linking the default profile.
	 * If AnimInstance is not yet ready, marks as pending and retries in Tick().
	 *
	 * @param InMeshComponent  目标骨骼网格组件
	 * @param InDefaultProfile 默认动画层配置
	 * @param InLinkProfiles   按 GameplayTag 映射的动画层配置
	 */
	void InitializeAnimLayerConfig(USkeletalMeshComponent* InMeshComponent, const FLAAnimLinkProfile& InDefaultProfile, const TMap<FGameplayTag, FLAAnimLinkProfile>& InLinkProfiles);

	/**
	 * 按 Tag 切换动画层
	 * Switch to the animation profile mapped to the given tag.
	 * Unlinks current profile, applies the new one, returns true if tag was found.
	 */
	UFUNCTION(Blueprintcallable, Category="Animation")
	bool LinkAnimInstanceByTag(FGameplayTag ProfileTag);

	/**
	 * 重置为默认动画层
	 * Unlink current profile and re-apply the default profile.
	 */
	UFUNCTION(Blueprintcallable, Category="Animation")
	void ResetAnimInstance();

	/**
	 * 按 Tag 查找配置
	 * Find the animation profile mapped to the given tag.
	 * Returns nullptr if the tag is not found in LinkProfiles.
	 */
	const FLAAnimLinkProfile* FindProfileByTag(FGameplayTag ProfileTag) const;

protected:
	/**
	 * 自动链接模式
	 * If true, the component will auto-link the DefaultProfile in BeginPlay
	 * using the owner's SkeletalMeshComponent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bAutoLink = false;

	/** 默认动画层配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	FLAAnimLinkProfile DefaultProfile;

	/** 按 GameplayTag 映射的动画层配置 (如: Weapon.Pistol, Weapon.Rifle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TMap<FGameplayTag, FLAAnimLinkProfile> LinkProfiles;

	/** 当前关联的骨骼网格组件 */
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<USkeletalMeshComponent> MeshComponent = nullptr;

	/** 当前激活的动画层配置 */
	UPROPERTY()
	FLAAnimLinkProfile CurrentProfile;

	/** 是否有待处理的链接请求 (AnimInstance 尚未就绪时为 true) */
	bool bPendingLink = false;

};
