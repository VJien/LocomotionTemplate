// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "LATypes.generated.h"

class UAnimSequence;
class UAnimInstance;

/**
 * 四方向枚举
 * Four cardinal directions used for directional animation selection.
 * Used by Start/Stop/Cycle/Pivot animations and additive hurt direction.
 */
UENUM(BlueprintType)
enum class ELADirection4: uint8
{
	Forward,
	Backward,
	Left,
	Right
};

/**
 * Root Yaw 偏移模式
 * Controls how RootYawOffset is updated per locomotion state.
 *
 * - BlendOut:  在移动状态下逐渐将偏移归零，使角色朝向跟随控制器朝向。
 *              Smoothly blend the yaw offset back to zero during movement.
 * - Hold:      在 Start 状态下保持进入时的偏移值不变。
 *              Preserve the offset value from when the state was entered (used during Start).
 * - Accumulate:在 Idle 状态下持续累积偏移，用于抵消 Pawn 旋转，保持脚部不滑动。
 *              Accumulate offset to counter Pawn rotation, keeping feet planted (used during Idle).
 */
UENUM(BlueprintType)
enum class ELARootYawOffsetMode: uint8
{
	BlendOut,
	Hold,
	Accumulate
};

/**
 * 步态类型
 * Character gait/movement speed category.
 * Maps to different animation sets and locomotion behaviors.
 *
 * - Walking:      慢走 (Walk)
 * - Running:      跑步/慢跑 (Jog)
 * - CrouchWalking: 蹲走 (Crouch Walk)
 * - Sprinting:    冲刺 (Sprint)
 */
UENUM(BlueprintType)
enum class ELAGait: uint8
{
	Walking,
	Running,
	CrouchWalking,
	Sprinting
};

/**
 * 数据验证标志位
 * Bitflags used by ULAAnimConfigDataAsset::ValidationFlags to control which animation sets
 * should be validated in the editor. Only flagged categories will have their animation
 * references and curves checked by IsDataValid().
 *
 * Usage: Set on the DataAsset's ValidationFlags property (bitmask in editor).
 * For example, a weapon without crouch animations should NOT set Crouch flag
 * to avoid validation errors on empty crouch animation slots.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ELADataValidationFlags: uint8
{
	None  = 0x00 UMETA(Hidden),
	Walk  = 1<<0,   // 启用 Walk (慢走) 动画验证 / Enable Walk animation validation
	Crouch = 1<<1,  // 启用 Crouch (蹲伏) 动画验证 / Enable Crouch animation validation
	Sprint = 1<<2,  // 启用 Sprint (冲刺) 动画验证 / Enable Sprint animation validation
	Jump = 1<<3,    // 启用 Jump (跳跃) 动画曲线验证 / Enable Jump curve validation

};
ENUM_CLASS_FLAGS(ELADataValidationFlags)

/**
 * 角色地面信息缓存
 * Cached ground detection result, updated once per frame.
 * Used primarily by the Jump/Fall state machine to determine landing distance.
 */
USTRUCT(BlueprintType)
struct FLACharacterGroundInfo
{
	GENERATED_BODY()

	FLACharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	/** 上次更新的帧号，用于避免同帧重复计算 / Frame counter to avoid redundant updates */
	uint64 LastUpdateFrame;

	/** 地面检测碰撞结果 / Hit result from ground detection trace */
	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	/** 角色到地面的距离 (厘米) / Distance from character to ground in cm. 0 = on ground */
	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * 四方向动画序列引用
 * Holds 4 directional UAnimSequence references (Forward/Backward/Left/Right).
 * Used by locomotion animations: Start, Stop, Cycle, Pivot.
 * Each direction maps to one cardinal direction animation.
 */
USTRUCT(BlueprintType)
struct FLAAnimSequence4
{
	GENERATED_BODY()

	/** 前进方向动画 / Forward direction */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimSequence* F = nullptr;
	/** 后退方向动画 / Backward direction */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
 	UAnimSequence* B = nullptr;
	/** 左移方向动画 / Left direction */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimSequence* L = nullptr;
	/** 右移方向动画 / Right direction */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimSequence* R = nullptr;

};

/**
 * 四方向动画序列数组
 * Same as FLAAnimSequence4 but each direction holds an array of animations
 * instead of a single one. Used for cases where random selection is needed,
 * such as additive hurt animations (multiple hit reactions per direction).
 */
USTRUCT(BlueprintType)
struct FLAAnimSequenceArray4
{
	GENERATED_BODY()

	/** 前进方向动画列表 / Forward direction animations */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<UAnimSequence*> F;
	/** 后退方向动画列表 / Backward direction animations */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<UAnimSequence*> B;
	/** 左移方向动画列表 / Left direction animations */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<UAnimSequence*> L;
	/** 右移方向动画列表 / Right direction animations */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<UAnimSequence*> R;

};


/**
 * 动画实例链接配置
 * Defines how a SkeletalMeshComponent's animation should be configured.
 * Supports two linking modes:
 * - ClassLinkedAnimInstances: 通过 LinkAnimClassLayers 绑定动画层 / Bind anim layers by class
 * - TagLinkedAnimInstances: 通过 LinkAnimGraphByTag 按 Tag 绑定 / Bind anim layers by tag
 *
 * Used by ULAAnimLinkComponent to dynamically switch animation layers
 * based on gameplay state (e.g., different weapon types, stances).
 *
 * Usage flow:
 * 1. Set DefaultProfile on the LinkComponent
 * 2. Optionally set LinkProfiles mapped by GameplayTag
 * 3. Call LinkAnimInstanceByTag() to switch profiles at runtime
 */
USTRUCT(BlueprintType)
struct FLAAnimLinkProfile
{
	GENERATED_BODY()

	/** 绑定动画层到 SkeletalMeshComponent / Link anim layers to the given mesh component */
	void Link(USkeletalMeshComponent* MeshComponent);
	/** 解绑动画层 / Unlink anim layers from the given mesh component */
	void Unlink(USkeletalMeshComponent* MeshComponent);
	/** 重置所有配置 / Reset all configuration */
	void Reset();

	/**
	 * 覆盖主 AnimInstance 类型
	 * Override the main AnimInstance class on the SkeletalMeshComponent.
	 * If set, replaces the component's AnimInstance when Link() is called.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UAnimInstance> OverrideAnimInstance = nullptr;

	/**
	 * 按类绑定动画层的列表
	 * List of anim layer classes to link via LinkAnimClassLayers().
	 * These are linked in order when Link() is called.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TSubclassOf<UAnimInstance>> ClassLinkedAnimInstances;

	/**
	 * 按 Tag 绑定动画层的映射
	 * Map of tag name -> anim layer class to link via LinkAnimGraphByTag().
	 * Key: Tag name in the AnimBP (e.g., "FullBody_Aiming")
	 * Value: AnimLayer class to link to that tag
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<FName, TSubclassOf<UAnimInstance>>  TagLinkedAnimInstances;

};
