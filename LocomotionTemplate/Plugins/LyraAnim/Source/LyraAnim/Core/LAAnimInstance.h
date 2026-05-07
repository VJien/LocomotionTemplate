// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "LAAdditiveHurtInterface.h"
#include "Animation/AnimInstance.h"
#include "LyraAnim/Data/LAAnimConfigDataAsset.h"
#include "LyraAnim/Data/LATypes.h"
#include "LAAnimInstance.generated.h"

class UAbilitySystemComponent;

/**
 * LyraAnim 动画实例基类
 *
 * 替代原版 Lyra 的 ULyraAnimInstance，提供:
 * 1. GameplayTag -> Blueprint变量 自动映射 (通过 GameplayTagPropertyMap)
 * 2. 地面距离检测 (GroundDistance)
 * 3. 受击叠加动画系统 (ILAAdditiveHurtInterface)
 * 4. 步态控制接口
 *
 * 使用方式:
 * - AnimBP (如 ABP_LA_Template) 继承此类
 * - 在 AnimConfig 属性上设置 ULAAnimConfigDataAsset 实例
 * - 通过 GameplayTagPropertyMap 绑定 GameplayTag 到蓝图变量
 * - BlueprintThreadSafeUpdateAnimation 中读取所有动画数据
 *
 * Replaces Lyra's ULyraAnimInstance with:
 * - GameplayTag property mapping for GAS integration
 * - Ground distance detection for jump/land state machine
 * - Additive hurt animation system
 * - Gait control interface
 */
UCLASS()
class LYRAANIM_API ULAAnimInstance : public UAnimInstance, public ILAAdditiveHurtInterface
{
	GENERATED_BODY()
public:

	ULAAnimInstance(const FObjectInitializer& ObjectInitializer);

	/**
	 * 使用 AbilitySystemComponent 初始化
	 * Initialize the GameplayTag property map from the given ASC.
	 * Called automatically in NativeInitializeAnimation if the owning actor has an ASC.
	 * Can also be called manually if ASC is set up after animation initialization.
	 */
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	//~ ILAAdditiveHurtInterface
	/**
	 * 应用受击叠加动画
	 * Trigger an additive hurt animation in the given direction.
	 * Alternates between bHurtA/bHurtB to allow re-triggering without waiting.
	 */
	virtual void ApplyAdditiveHurt_Implementation(ELADirection4 Direction) override;

	/**
	 * 停止受击叠加动画
	 * Stop the current additive hurt animation by resetting both triggers.
	 */
	virtual void StopAdditiveHurt_Implementation() override;
	//~ End ILAAdditiveHurtInterface

	/**
	 * 设置期望步态
	 * Set the desired gait for the character.
	 * Used by BlueprintThreadSafeUpdateAnimation to determine animation set selection.
	 */
	UFUNCTION(BlueprintCallable)
	void SetGait(ELAGait InTargetGait);

protected:

#if WITH_EDITOR
	/** 编辑器数据验证 - 检查 GameplayTag 映射是否有效 */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

	/**
	 * 获取地面信息 (带缓存)
	 * Returns cached ground info, updated once per frame.
	 * Performs line trace downward when character is not on ground.
	 */
	const FLACharacterGroundInfo& GetGroundInfo();

	//~ UAnimInstance
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	//~ End UAnimInstance

protected:

	// =========================================================================
	// Config / 配置
	// =========================================================================

	/**
	 * GameplayTag -> 蓝图变量 映射
	 * Maps GameplayTags to boolean/integer/float blueprint properties.
	 * Properties are automatically updated as tags are added/removed on the ASC.
	 * Should be used instead of manually querying tags every frame.
	 *
	 * 配置方式: 在 AnimBP 的 Class Defaults -> Gameplay Tags -> Gameplay Tag Property Map
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	/**
	 * 动画配置数据资产
	 * Reference to the animation configuration DataAsset.
	 * All animation references and behavior parameters are read from this asset.
	 * Set different DataAssets for different weapon types / character stances.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	TObjectPtr<ULAAnimConfigDataAsset> AnimConfig = nullptr;

	// =========================================================================
	// Character State Data / 角色状态数据
	// =========================================================================

	/**
	 * 角色到地面距离 (厘米)
	 * Distance from character to ground, updated every frame.
	 * - 0.0 = on ground (walking)
	 * - >0.0 = airborne distance
	 * - -1.0 = not yet calculated
	 * Used by Jump/Fall state machine to trigger FallLand when < threshold (default 200cm).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float GroundDistance = -1.0f;

	/** 缓存的地面信息 (内部使用，通过 GetGroundInfo() 访问) */
	FLACharacterGroundInfo CachedGroundInfo;

	/**
	 * 地面检测最大追踪距离 (厘米)
	 * Maximum distance for the downward line trace used to detect ground.
	 */
	float GroundTraceDistance = 100000.0f;

	// =========================================================================
	// Hurt State / 受击状态
	// =========================================================================

	/**
	 * 受击触发器 A
	 * Alternating trigger for additive hurt animations.
	 * When a hurt is applied, toggles between A and B to allow re-triggering
	 * without waiting for the previous hurt to finish.
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Hurt")
	bool bHurtA = false;

	/** 受击触发器 B (与 bHurtA 交替使用) */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Hurt")
	bool bHurtB = false;

	/** 当前受击方向 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Hurt")
	ELADirection4 HurtDirection = ELADirection4::Forward;

	// =========================================================================
	// Movement / 移动状态
	// =========================================================================

	/**
	 * 期望步态
	 * The desired gait set by gameplay code via SetGait().
	 * Used by the AnimBP to select appropriate animation sets (Walk/Jog/Crouch/Sprint).
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Movement")
	ELAGait DesireGait = ELAGait::Running;

};
