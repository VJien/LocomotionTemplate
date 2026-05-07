// Fill out your copyright notice in the Description page of Project Common.

#pragma once

#include "CoreMinimal.h"
#include "LATypes.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/BlendProfile.h"
#include "Engine/DataAsset.h"
#include "LAAnimConfigDataAsset.generated.h"




/**
 * LyraAnim 动画配置数据资产
 *
 * 核心设计思想:
 * 将原版 Lyra 动画系统中每个武器需要单独子 AnimBP (Linked Layer Child) 的模式，
 * 替换为数据驱动模式 — 每个武器/姿态只需创建一个新的 DataAsset 实例即可。
 *
 * Core design:
 * Replaces Lyra's per-weapon Child AnimBP pattern with a data-driven approach.
 * Instead of creating a new Linked Layer Child Blueprint for each weapon,
 * simply create a new DataAsset instance with the appropriate animation references.
 *
 * 资产结构分为以下几大类:
 * 1. Debug       - 调试与验证配置
 * 2. Settings    - 运行时行为参数 (Root Yaw、移动、瞄准、步幅扭曲、播放速率)
 * 3. Curve       - 动画曲线名称映射 (TurnInPlace、距离匹配、IK)
 * 4. Animation   - 各状态动画资产引用 (Idle/Start/Stop/Cycle/Pivot/Sprint/TurnInPlace/Jump/Aim/Lean/Hurt)
 *
 * 使用方式:
 * - 在 ULAAnimInstance::AnimConfig 上引用此资产
 * - ABP_LA_Template 通过 AnimConfig 读取所有动画和参数
 * - 不同武器/姿态创建不同的 DataAsset 实例 (如 DA_AnimConfig_ue5_pistol, DA_AnimConfig_ue5_unarmed)
 */
UCLASS(BlueprintType)
class LYRAANIM_API ULAAnimConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
public:
	ULAAnimConfigDataAsset();


	// =========================================================================
	// Debug / 调试配置
	// =========================================================================

	/**
	 * 数据验证标志位 (位掩码)
	 * Bitmask controlling which animation sets are validated by IsDataValid().
	 * Only flagged categories will be checked for missing references and curves.
	 * For example, if a weapon has no crouch animations, don't set the Crouch flag.
	 *
	 * 可选值: Walk, Crouch, Sprint, Jump
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Debug", meta=(Bitmask, BitmaskEnum="/Script/LyraAnim.ELADataValidationFlags"))
	int32 ValidationFlags;


	// =========================================================================
	// Settings / 运行时行为参数
	// =========================================================================

	// --- Root Yaw Offset ---

	/**
	 * 启用 Root Yaw 偏移 (TurnInPlace 的基础)
	 * Enable Root Yaw Offset system. When enabled, the character's mesh rotation
	 * is countered during idle to keep feet planted, and TurnInPlace animations
	 * are triggered when the offset exceeds the threshold.
	 * 关闭后 TurnInPlace 功能将不会生效。
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|RootYaw")
	bool bEnableRootYawOffset = true;

	/**
	 * 站立状态 Root Yaw 偏移角度钳制范围 (度)
	 * Min/Max clamp for RootYawOffset during standing idle.
	 * Prevents over-twisting of the spine when aiming too far backwards.
	 * If the player rotates the camera too quickly, clamping will cause foot sliding.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|RootYaw", meta=(EditCondition="bEnableRootYawOffset", EditConditionHides))
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.f, 100.f);

	/**
	 * 蹲伏状态 Root Yaw 偏移角度钳制范围 (度)
	 * Same as RootYawOffsetAngleClamp but for crouching idle.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|RootYaw", meta=(EditCondition="bEnableRootYawOffset", EditConditionHides))
	FVector2D RootYawOffsetAngleClampCrouch = FVector2D(-120.f, 100.f);

		/**
		 * TurnInPlace 触发角度阈值 (度)
		 * When |RootYawOffset| exceeds this value during Idle, a TurnInPlace animation is triggered.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|RootYaw", meta=(EditCondition="bEnableRootYawOffset", EditConditionHides))
		float TurnInPlaceAngleThreshold = 50.f;

		/**
		 * Start -> Cycle 的 RootYawOffset 触发阈值 (度)
		 * When |RootYawOffset| exceeds this value during Start state, immediately transition to Cycle.
		 * Prevents holding a start animation while facing too far from movement direction.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|RootYaw", meta=(EditCondition="bEnableRootYawOffset", EditConditionHides))
		float StartToCycleRootYawThreshold = 60.f;

	// --- Locomotion / 移动 ---

	/**
	 * 基数方向死区角度 (度)
	 * Dead zone angle for cardinal direction classification (Forward/Backward/Left/Right).
	 * Within this angle from a cardinal direction, the animation won't switch.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
	float CardinalDirectionDeadZone = 10.f;

	/**
	 * 启用脚步放置 IK
	 * Enable Foot Placement node to plant feet on uneven terrain.
	 * When disabled, the FootPlacement node is bypassed in the AnimGraph.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
	bool bUseFootPlacement = false;

	/**
	 * 跟随速度方向移动时仅使用前向动画
	 * When using orientation-based movement (velocity direction mode),
	 * only use Forward animations and rely on Orientation Warping to fill gaps.
	 * If false, all 4 cardinal direction animations are used.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
	bool bOnlyForwardAnimWhenMoveOrientation = true;

		/**
		 * Idle Break 触发倒计时 (秒)
		 * Time in seconds the character must be idle before an Idle Break animation plays.
		 * Timer resets on any locomotion state change.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
		float TimeUntilIdleBreak = 5.f;

		/**
		 * Start 状态最大停留时间 (秒)
		 * If the character stays in Start state longer than this AND
		 * DisplacementSpeed < StartDisplacementSpeedThreshold, auto-transition to Cycle.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
		float StartStateMaxTime = 0.15f;

		/**
		 * Start 状态位移速度阈值 (cm/s)
		 * Below this speed, Start is considered failed/stuck and will transition out.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Locomotion")
		float StartDisplacementSpeedThreshold = 10.f;

	// --- Aiming / 瞄准 ---

	/**
	 * 启用瞄准时 HipFire 姿势覆盖
	 * Allow overriding the standing pose when aiming (HipFire vs ADS).
	 * When false, ADS pose is always used while aiming, which may look poor in some cases.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Aiming")
	bool bEnableHipfirePoseOverride = true;

	/**
	 * 蹲伏射击后抬枪恢复
	 * When crouched and firing, blend back to aiming pose after firing stops.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Aiming")
	bool bRaiseWeaponAfterFiringWhenCrouched = true;

	/**
	 * 蹲伏射击后抬枪恢复持续时间 (秒)
	 * Duration for blending back to aiming pose after crouch firing.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Aiming", meta=(EditCondition="bRaiseWeaponAfterFiringWhenCrouched", EditConditionHides))
	float RaiseWeaponAfterFiringDuration = 0.5f;

	/**
	 * ADS 时是否总是使用走路步态
	 * If true, ADS (Aim Down Sights) is always linked to Walk gait.
	 * If false, ADS is decoupled from gait — player can ADS while jogging.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Aiming")
	bool bAdsIsAlwaysWalking = false;

	// --- IK ---

	/**
	 * 禁用手部 IK
	 * Disable hand IK (TwoBoneIK nodes for left/right hand).
	 * When true, HandIKLeftAlpha and HandIKRightAlpha are forced to 0.
	 * Useful when weapon doesn't need hand IK correction.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|IK")
	bool bDisableHandIK = true;

	/**
	 * 启用左手姿势覆盖
	 * Enable left hand finger pose override via LayeredBlendPerBone.
	 * Used to adjust left hand finger poses for different weapon grips.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|IK")
	bool bEnableLeftHandPoseOverride = false;

	// --- Stride Warping / 步幅扭曲 ---

	/**
	 * 步幅扭曲混合持续时间 (按比例缩放)
	 * Duration for blending in Stride Warping, scaled by transition progress.
	 * Start states begin with Distance Matching, then gradually blend in Stride Warping.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Warping")
	float StrideWarpingBlendInDurationScaled = 0.2f;

	/**
	 * 步幅扭曲混合开始偏移 (比例)
	 * Start offset (as fraction) within the animation when Stride Warping begins blending in.
	 * E.g., 0.15 means Stride Warping starts blending in at 15% of the start animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Warping")
	float StrideWarpingBlendInStartOffset = 0.15f;

		/**
		 * Orientation Warping 旋转插值速度
		 * Interpolation speed for Orientation Warping to rotate lower body toward movement direction.
		 * Higher = snappier response, Lower = smoother transition.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Warping")
		float OrientationWarpingInterpSpeed = 10.f;

	// --- PlayRate / 播放速率 ---

	/**
	 * 循环动画 (Cycle) 播放速率钳制范围
	 * Min/Max play rate clamp for cycle animations (Jog/Walk/Crouch loops).
	 * Prevents animation from playing too slow or too fast when matching speed.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|PlayRate")
	FVector2D PlayRateClampCycle = FVector2D(0.8F, 1.2f);

	/**
	 * 起止动画 (Start/Stop) 播放速率钳制范围
	 * Min/Max play rate clamp for start and stop animations.
	 * Wider range than Cycle since Distance Matching needs more flexibility.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|PlayRate")
	FVector2D PlayRateClampStart = FVector2D(0.6f, 2.f);

	// --- Jump ----

		/**
		 * Landing distance threshold (cm)
		 * When GroundDistance < this value during FallLoop, transition to FallLand state.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Jump")
		float FallLandGroundDistanceThreshold = 200.f;

		/**
		 * Time to jump apex threshold (seconds)
		 * When TimeToJumpApex < this value during JumpStartLoop, transition to JumpApex.
		 * TimeToJumpApex is calculated as: -WorldVelocity.Z / GravityZ.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Jump")
		float TimeToJumpApexThreshold = 0.4f;

	// --- Blend ----

		/**
		 * Upper body blend profile
		 * Blend profile for upper/lower body split (LayeredBlendPerBone).
		 * Defines gradual weight transition from Spine1 to Arm bones.
		 * If null, a hardcoded or default profile will be used.
		 */
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Blend")
		TObjectPtr<UBlendProfile> UpperBodyBlendProfile;


	// =========================================================================
	// Curve / 动画曲线名称配置
	// 这些名称用于在运行时从动画资产中读取对应的曲线值。
	// 如果你的动画使用了不同的曲线命名约定，可以在这里修改映射。
	// =========================================================================

	/**
	 * TurnInPlace 旋转权重曲线
	 * Curve in TurnInPlace animations indicating rotation weight (0-1).
	 * When value reaches 0, transition from TurnInPlace to TurnInPlaceRecovery.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName TurnYawWeightCurveName = "TurnYawWeight";

	/**
	 * TurnInPlace 剩余旋转角度曲线
	 * Curve in TurnInPlace animations indicating remaining yaw to rotate.
	 * Used to accumulate RootYawOffset during TurnInPlace playback.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName RemainingTurnYawCurveName = "RemainingTurnYaw";

	/**
	 * 跳跃/着陆距离匹配曲线
	 * Curve in Jump_Land animation for distance matching towards landing target.
	 * The Jump_Land animation is speed-adjusted to match this distance curve.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName JumpDistanceCurveName = "GroundDistance";

	/**
	 * 移动动画距离曲线
	 * Distance curve in locomotion animations (Start/Stop/Pivot).
	 * Used by Distance Matching to sync animation with actual movement distance.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName LocomotionDistanceCurveName = "Distance";

	/**
	 * 禁用右手 IK 曲线
	 * Curve to disable right hand IK (0 = enabled, 1 = disabled).
	 * Typically used in equipment/unequipment animations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName DisableRHandIKCurveName = "DisableRightHandIK";

	/**
	 * 禁用左手 IK 曲线
	 * Curve to disable left hand IK (0 = enabled, 1 = disabled).
	 * Typically used in equipment/unequipment and melee animations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName DisableLHandIKCurveName = "DisableLeftHandIK";

	/**
	 * 禁用腿部 IK 曲线
	 * Curve to disable leg IK / FootPlacement (0 = enabled, 1 = disabled).
	 * Typically used in dash animations where the character is airborne.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curve")
	FName DisableLegIKCurveName = "DisableLegIK";


	// =========================================================================
	// Animation / 动画资产引用
	// =========================================================================

	// --- Idle / 待机 ---

	/**
	 * 待机 HipFire 姿势动画
	 * Idle pose for hip-fire stance (weapon lowered, not aiming).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TObjectPtr<UAnimSequence> Idle_Hipfire = nullptr;

	/**
	 * 待机 ADS 瞄准姿势动画
	 * Idle pose for ADS (Aim Down Sights) stance.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TObjectPtr<UAnimSequence> Idle_ADS = nullptr;

	/**
	 * 待机动画叠加权重
	 * Blend weight for additive idle pose (e.g., breathing, subtle weight shifts).
	 * 0 = no additive, 1 = full additive.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	float Idle_AdditiveWeight = 0.f;

	/**
	 * 待机休息动画列表
	 * Array of idle break animations. Played randomly after character is idle
	 * for TimeUntilIdleBreak seconds. Provides natural "waiting" variation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TArray<TObjectPtr<UAnimSequence>> Idle_Breaks;

	/**
	 * 蹲伏待机循环动画
	 * Crouching idle loop animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TObjectPtr<UAnimSequence> Crouch_Idle = nullptr;

	/**
	 * 蹲伏待机进入动画
	 * Transition animation into crouch idle (played once when entering crouch).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TObjectPtr<UAnimSequence> Crouch_Idle_Entry = nullptr;

	/**
	 * 蹲伏待机退出动画
	 * Transition animation out of crouch idle (played once when standing up).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Idle")
	TObjectPtr<UAnimSequence> Crouch_Idle_Exit = nullptr;

	// --- Pose Override / 姿势覆盖 ---

	/**
	 * 左手姿势覆盖动画
	 * Left hand finger pose override animation.
	 * First frame is extracted and blended onto the left hand fingers
	 * via LayeredBlendPerBone to adjust grip for different weapons.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Pose")
	TObjectPtr<UAnimSequence> LeftHand_Pose = nullptr;

	// --- Start / 启动动画 ---

	/**
	 * 启用方向启动动画
	 * Enable directional start animations. When disabled, transitions
	 * directly from Idle to Cycle without a start animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Start")
	bool bEnableStartAnims = true;

	/**
	 * 启用启动动画距离匹配
	 * Use Distance Matching for start animations.
	 * Adjusts play rate so the animation's distance curve matches actual displacement.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Start", meta=(EditCondition="bEnableStartAnims", EditConditionHides))
	bool bEnableStartDistanceMatching = true;

	/**
	 * 慢跑启动动画 (四方向)
	 * Jog/Run start animations for 4 cardinal directions.
	 * Also used as ADS start animations when ADS is active.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Start", meta=(EditCondition="bEnableStartAnims", EditConditionHides))
	FLAAnimSequence4 Jog_Start;

	/**
	 * 慢走启动动画 (四方向)
	 * Walk start animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Start", meta=(EditCondition="bEnableStartAnims", EditConditionHides))
	FLAAnimSequence4 Walk_Start;

	/**
	 * 蹲走启动动画 (四方向)
	 * Crouch walk start animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Start", meta=(EditCondition="bEnableStartAnims", EditConditionHides))
	FLAAnimSequence4 Crouch_Start;


	// --- Stop / 停止动画 ---

	/**
	 * 启用方向停止动画
	 * Enable directional stop animations. When disabled, transitions
	 * directly from locomotion to Idle without a stop animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop")
	bool bEnableStopAnims = true;

	/**
	 * 启用停止动画距离匹配
	 * Use Distance Matching for stop animations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop", meta=(EditCondition="bEnableStopAnims", EditConditionHides))
	bool bEnableStopDistanceMatching = true;

	/**
	 * 停止距离匹配速度阈值
	 * Speed threshold below which Distance Matching is used for stop animations.
	 * Above this speed, the stop animation plays at normal rate.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop", meta=(EditCondition="bEnableStopAnims", EditConditionHides))
	float StopDistanceMatchSpeedThreshold = 5.f;

	/**
	 * 慢跑停止动画 (四方向)
	 * Jog/Run stop animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop", meta=(EditCondition="bEnableStopAnims", EditConditionHides))
	FLAAnimSequence4 Jog_Stop;

	/**
	 * 慢走停止动画 (四方向)
	 * Walk stop animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop", meta=(EditCondition="bEnableStopAnims", EditConditionHides))
	FLAAnimSequence4 Walk_Stop;

	/**
	 * 蹲走停止动画 (四方向)
	 * Crouch walk stop animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Stop", meta=(EditCondition="bEnableStopAnims", EditConditionHides))
	FLAAnimSequence4 Crouch_Stop;


	// --- Cycle / 循环移动动画 ---

	/**
	 * 使用 BlendSpace 作为循环动画
	 * When true, uses BlendSpace assets instead of individual AnimSequences for cycles.
	 * BlendSpace allows smooth interpolation between directions/speeds.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle")
	bool bUseBlendSpaceCycle = false;

	/**
	 * 慢跑循环动画 (四方向, 非 BlendSpace 模式)
	 * Jog/Run cycle animations for 4 cardinal directions.
	 * Only used when bUseBlendSpaceCycle is false.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bUseBlendSpaceCycle", EditConditionHides))
	FLAAnimSequence4 Jog_Cycle;

	/**
	 * 慢跑循环动画是否包含 Root Motion
	 * If true, speed is extracted from Root Motion. If false, use JogCycleSpeed.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle")
	bool bJogCycleAnimHasRootMotion = true;

	/**
	 * 慢跑循环动画手动速度 (无 Root Motion 时使用)
	 * Authoring speed of the jog cycle animation when it has no Root Motion.
	 * Used for SetPlayrateToMatchSpeed calculations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bJogCycleAnimHasRootMotion", EditConditionHides))
	float JogCycleSpeed = 600.f;

	/**
	 * 慢走循环动画 (四方向, 非 BlendSpace 模式)
	 * Walk cycle animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bUseBlendSpaceCycle", EditConditionHides))
	FLAAnimSequence4 Walk_Cycle;

	/**
	 * 慢走循环动画是否包含 Root Motion
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bUseBlendSpaceCycle", EditConditionHides))
	bool bWalkCycleAnimHasRootMotion = true;

	/**
	 * 慢走循环动画手动速度 (无 Root Motion 时使用)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bWalkCycleAnimHasRootMotion", EditConditionHides))
	float WalkCycleSpeed = 300.f;

	/**
	 * 蹲走循环动画 (四方向, 非 BlendSpace 模式)
	 * Crouch walk cycle animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bUseBlendSpaceCycle", EditConditionHides))
	FLAAnimSequence4 Crouch_Cycle;

	/**
	 * 蹲走循环动画是否包含 Root Motion
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bUseBlendSpaceCycle", EditConditionHides))
	bool bCrouchCycleAnimHasRootMotion = true;

	/**
	 * 蹲走循环动画手动速度 (无 Root Motion 时使用)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="!bCrouchCycleAnimHasRootMotion", EditConditionHides))
	float CrouchCycleSpeed = 200.f;

	/**
	 * BlendSpace 模式最大循环速度
	 * Maximum speed value for BlendSpace cycle animations.
	 * Only used when bUseBlendSpaceCycle is true.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="bUseBlendSpaceCycle", EditConditionHides))
	float MaxCycleSpeed = 600.f;

	/**
	 * 站立循环 BlendSpace
	 * BlendSpace for standing cycle locomotion (replaces Jog_Cycle + Walk_Cycle).
	 * Should have speed axis mapped from Walk to Jog.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="bUseBlendSpaceCycle", EditConditionHides))
	TObjectPtr<UBlendSpace> BS_Stand_Cycle = nullptr;

	/**
	 * 蹲伏循环 BlendSpace
	 * BlendSpace for crouching cycle locomotion (replaces Crouch_Cycle).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Cycle", meta=(EditCondition="bUseBlendSpaceCycle", EditConditionHides))
	TObjectPtr<UBlendSpace> BS_Crouch_Cycle = nullptr;

	// --- Pivot / 转向动画 ---

	/**
	 * 启用转向动画
	 * Enable pivot animations. Played when the character makes a drastic
	 * direction change (velocity dot acceleration < 0) while moving.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Pivot")
	bool bEnablePivotAnim = true;

	/**
	 * 慢跑转向动画 (四方向)
	 * Jog pivot animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Pivot", meta=(EditCondition="bEnablePivotAnim", EditConditionHides))
	FLAAnimSequence4 Jog_Pivot;

	/**
	 * 慢走转向动画 (四方向)
	 * Walk pivot animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Pivot", meta=(EditCondition="bEnablePivotAnim", EditConditionHides))
	FLAAnimSequence4 Walk_Pivot;

	/**
	 * 蹲走转向动画 (四方向)
	 * Crouch walk pivot animations for 4 cardinal directions.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Pivot", meta=(EditCondition="bEnablePivotAnim", EditConditionHides))
	FLAAnimSequence4 Crouch_Pivot;


	// --- Sprint / 冲刺动画 ---

	/**
	 * 启用冲刺进入动画
	 * Play a transition animation when entering sprint state.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint")
	bool bEnableSprintEntry = false;

	/**
	 * 冲刺进入动画
	 * Transition animation from jog/run into sprint.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint", meta=(EditCondition="bEnableSprintEntry", EditConditionHides))
	TObjectPtr<UAnimSequence> Sprint_Entry = nullptr;

	/**
	 * 启用冲刺退出动画
	 * Play a transition animation when exiting sprint state.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint")
	bool bEnableSprintExit = false;

	/**
	 * 冲刺退出动画
	 * Transition animation from sprint back to jog/run.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint", meta=(EditCondition="bEnableSprintExit", EditConditionHides))
	TObjectPtr<UAnimSequence> Sprint_Exit = nullptr;

	/**
	 * 冲刺循环动画
	 * Sprint loop cycle animation (forward only, typically no directional variants).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint")
	TObjectPtr<UAnimSequence> Sprint_Loop = nullptr;

	/**
	 * 冲刺循环叠加动画
	 * Additive animation applied on top of Sprint_Loop for additional motion (e.g., weapon sway).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint")
	TObjectPtr<UAnimSequence> Sprint_Loop_Additive = nullptr;

	/**
	 * 冲刺循环动画是否包含 Root Motion
	 * If true, speed is extracted from Root Motion. If false, use SprintLoopSpeed.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint")
	bool bSprintLoopAnimHasRootMotion = true;

	/**
	 * 冲刺循环动画手动速度 (无 Root Motion 时使用)
	 * Authoring speed for sprint loop when it has no Root Motion.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Sprint", meta=(EditCondition="!bSprintLoopAnimHasRootMotion", EditConditionHides))
	float SprintLoopSpeed = 800.f;

	// --- TurnInPlace / 原地转向 ---

	/**
	 * 左转原地转向动画 (站立)
	 * Turn-in-place animation for rotating left while standing idle.
	 * Must have TurnYawWeight and RemainingTurnYaw curves baked in.
	 * Requires Root Motion enabled on the source animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|TurnInPlace")
	TObjectPtr<UAnimSequence> TurnInPlace_Left;

	/**
	 * 右转原地转向动画 (站立)
	 * Turn-in-place animation for rotating right while standing idle.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|TurnInPlace")
	TObjectPtr<UAnimSequence> TurnInPlace_Right;

	/**
	 * 左转原地转向动画 (蹲伏)
	 * Turn-in-place animation for rotating left while crouching idle.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|TurnInPlace")
	TObjectPtr<UAnimSequence> Crouch_TurnInPlace_Left;

	/**
	 * 右转原地转向动画 (蹲伏)
	 * Turn-in-place animation for rotating right while crouching idle.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|TurnInPlace")
	TObjectPtr<UAnimSequence> Crouch_TurnInPlace_Right;


	// --- Jump / 跳跃动画 ---

	/**
	 * 启用完整跳跃动画序列
	 * If true, uses full jump sequence: Start -> StartLoop -> Apex -> FallLoop -> Land.
	 * If false, uses simplified: Start -> FallLoop -> Land.
	 * Set to false for weapons/characters without full jump animation coverage.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	bool bEnableFullJumpAnim = true;

	/**
	 * 跳跃启动动画
	 * Initial jump launch animation (plays once when jump starts).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	TObjectPtr<UAnimSequence> Jump_Start = nullptr;

	/**
	 * 跳跃启动动画播放起始位置 (归一化时间)
	 * Normalized time position to start playing Jump_Start animation.
	 * Useful for skipping the beginning of the animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	float JumpStartPosition = 0.0f;

	/**
	 * 跳跃上升循环动画
	 * Looping animation during the ascending phase of the jump.
	 * Only used when bEnableFullJumpAnim is true.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump", meta=(EditCondition="bEnableFullJumpAnim", EditConditionHides))
	TObjectPtr<UAnimSequence> Jump_Start_Loop = nullptr;

	/**
	 * 跳跃最高点动画
	 * Animation played at the apex of the jump (transition from ascending to falling).
	 * Only used when bEnableFullJumpAnim is true.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump", meta=(EditCondition="bEnableFullJumpAnim", EditConditionHides))
	TObjectPtr<UAnimSequence> Jump_Apex = nullptr;

	/**
	 * 下落循环动画是否循环播放
	 * Whether Jump_Fall_Loop should loop. Set to false for one-shot fall animations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	bool bJumpFallLoopAnimIsLooping = true;

	/**
	 * 下落循环动画
	 * Looping animation during free-fall phase.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	TObjectPtr<UAnimSequence> Jump_Fall_Loop = nullptr;

	/**
	 * 着陆动画
	 * Landing animation, uses distance matching (GroundDistance curve)
	 * to sync with the approaching ground.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	TObjectPtr<UAnimSequence> Jump_Land = nullptr;

	/**
	 * 启用着陆恢复叠加动画
	 * Apply an additive recovery animation after landing for a more natural settle.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump")
	bool bEnableLandAdditive = true;

	/**
	 * 着陆恢复叠加动画
	 * Additive animation applied after landing to add recovery motion
	 * (e.g., knees bending, body settling). Different per weapon type.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Jump", meta=(EditCondition="bEnableLandAdditive", EditConditionHides))
	TObjectPtr<UAnimSequence> Jump_RecoveryAdditive = nullptr;

	// --- Aim / 瞄准偏移 ---

	/**
	 * 启用瞄准偏移 (AimOffset BlendSpace)
	 * Enable aim offset system. When disabled, no aiming pose correction is applied.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Aim")
	bool bEnableAimOffset = true;

	/**
	 * HipFire 站立瞄准姿势
	 * Standing hip-fire aim pose. Used as base pose for relaxed (non-ADS) aiming,
	 * and blended with jump animations to hold weapon during air time.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Aim", meta=(EditCondition="bEnableAimOffset", EditConditionHides))
	TObjectPtr<UAnimSequence> Aim_HipFirePose = nullptr;

	/**
	 * HipFire 蹲伏瞄准姿势
	 * Crouching hip-fire aim pose. Same purpose as Aim_HipFirePose but for crouch state.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Aim", meta=(EditCondition="bEnableAimOffset", EditConditionHides))
	TObjectPtr<UAnimSequence> Aim_HipFirePose_Crouch = nullptr;

	/**
	 * 待机放松状态瞄准偏移 BlendSpace
	 * AimOffset BlendSpace for idle relaxed (non-ADS) stance.
	 * Maps pitch/yaw to aiming pose offsets.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Aim", meta=(EditCondition="bEnableAimOffset", EditConditionHides))
	TObjectPtr<UAimOffsetBlendSpace> IdleAimOffset_Relaxed = nullptr;

	/**
	 * ADS 瞄准状态瞄准偏移 BlendSpace
	 * AimOffset BlendSpace for ADS (Aim Down Sights) stance.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Aim", meta=(EditCondition="bEnableAimOffset", EditConditionHides))
	TObjectPtr<UAimOffsetBlendSpace> IdleAimOffset_ADS = nullptr;

	// --- Lean / 身体倾斜 ---

	/**
	 * 慢跑身体倾斜 BlendSpace (1D)
	 * BlendSpace1D for body lean during jog. Driven by yaw rotation speed
	 * to add natural leaning when turning while running.
	 * Input value: yaw rotation rate (degrees/sec).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Lean")
	TObjectPtr<UBlendSpace1D> BS_Jog_Lean = nullptr;

	/**
	 * 倾斜动画权重
	 * Blend weight for the lean additive pose (0 = no lean, 1 = full lean).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|Lean")
	float LeanWeight = 1.0f;

	// --- AdditiveHurt / 受击叠加动画 ---

	/**
	 * 启用受击叠加动画
	 * Enable directional additive hurt animations.
	 * When a character takes damage, a random hurt animation from the matching
	 * direction is blended on top of the current pose.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt")
	bool bEnableAdditiveHurts = true;

	/**
	 * 仅上半身受击叠加
	 * If true, hurt additive animations only affect the upper body (spine and above).
	 * Prevents hurt reactions from interfering with lower body locomotion.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	bool bOnlyUpperBodyAdditiveHurts = true;

	/**
	 * 计算受击方向
	 * If true, determines hit direction from damage source location.
	 * If false, always uses the Forward direction hurt animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	bool bCalcAdditiveHurtDirection = true;

	/**
	 * 四方向受击叠加动画列表
	 * Arrays of additive hurt animations per direction.
	 * A random animation is selected from the matching direction array on each hit.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	FLAAnimSequenceArray4 AdditiveHurts;

	/**
	 * 受击动画混合淡出比例 (0-1)
	 * Fraction of the hurt animation duration at which to start blending out.
	 * E.g., 0.8 = start fading out at 80% of the animation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	float AdditiveHurtBlendOutFrac = 0.8f;

	/**
	 * 受击动画播放速率
	 * Play rate multiplier for hurt additive animations.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	float AdditiveHurtPlayRate = 1.0f;

	/**
	 * 受击动画叠加权重
	 * Blend weight for the hurt additive pose (0 = no hurt, 1 = full hurt reaction).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Animation|AdditiveHurt", meta=(EditCondition="bEnableAdditiveHurts", EditConditionHides))
	float AdditiveHurtWeight = 1.0f;

};
