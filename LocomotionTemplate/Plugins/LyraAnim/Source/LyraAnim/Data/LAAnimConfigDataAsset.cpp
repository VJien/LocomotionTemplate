// Fill out your copyright notice in the Description page of Project Settings.


#include "LAAnimConfigDataAsset.h"

#include "LASettings.h"
#include "Animation/AnimSequence.h"
#include "Misc/DataValidation.h"
#include "LASettings.h"

#if WITH_EDITOR
#include "Editor/AnimationBlueprintLibrary/Public/AnimationBlueprintLibrary.h"
#include "AnimationGraph.h"
#endif

ULAAnimConfigDataAsset::ULAAnimConfigDataAsset()
{
}

#if WITH_EDITOR
EDataValidationResult ULAAnimConfigDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	const ULASettings* Settings = ULASettings::Get();
	if (Settings && !Settings->bEnableDataAssetValidation)
	{
		return EDataValidationResult::Valid;
	}
	
	const bool bNeedWalkValidation =  (ValidationFlags & static_cast<uint8>(ELADataValidationFlags::Walk)) != 0;
	const bool bNeedCrouchValidation = (ValidationFlags & static_cast<uint8>(ELADataValidationFlags::Crouch)) != 0;
	const bool bNeedSprintValidation =  (ValidationFlags & static_cast<uint8>(ELADataValidationFlags::Sprint)) != 0;
	const bool bNeedJumpValidation =  (ValidationFlags & static_cast<uint8>(ELADataValidationFlags::Jump)) != 0;

	
	EDataValidationResult Result = Super::IsDataValid(Context);

	// --- Animation Slot Validation ---
	const auto CheckAnim = [&](const UObject* Anim, const FString& SlotName) {
		if (Anim == nullptr)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString(FString::Printf(TEXT("Animation slot '%s' is not set."), *SlotName))); 
		}
	};

	const auto CheckAnimGroup = [&](const FLAAnimSequence4& AnimGroup, const FString& GroupName) {
		CheckAnim(AnimGroup.F, GroupName + TEXT(".F"));
		CheckAnim(AnimGroup.B, GroupName + TEXT(".B"));
		CheckAnim(AnimGroup.L, GroupName + TEXT(".L"));
		CheckAnim(AnimGroup.R, GroupName + TEXT(".R"));
	};

	if (bEnableRootYawOffset) {
		
			CheckAnim(TurnInPlace_Left, TEXT("TurnInPlace_Left"));
			CheckAnim(TurnInPlace_Right, TEXT("TurnInPlace_Right"));
		
		if (bNeedCrouchValidation)
		{
			CheckAnim(Crouch_TurnInPlace_Left, TEXT("Crouch_TurnInPlace_Left"));
			CheckAnim(Crouch_TurnInPlace_Right, TEXT("Crouch_TurnInPlace_Right"));
		}
		
	}

	if (bEnableStartAnims) {
		
		CheckAnimGroup(Jog_Start, TEXT("Jog_Start"));
		
		if (bNeedWalkValidation)
		{
			CheckAnimGroup(Walk_Start, TEXT("Walk_Start"));
		}
		if (bNeedCrouchValidation)
		{
			CheckAnimGroup(Crouch_Start, TEXT("Crouch_Start"));
		}
	}

	if (bEnableStopAnims) {
		
		CheckAnimGroup(Jog_Stop, TEXT("Jog_Stop"));
		if (bNeedWalkValidation)
		{
			CheckAnimGroup(Walk_Stop, TEXT("Walk_Stop"));
		}
		if (bNeedCrouchValidation)
		{
			CheckAnimGroup(Crouch_Stop, TEXT("Crouch_Stop"));
		}
	}

	if (bEnablePivotAnim) {
		
		CheckAnimGroup(Jog_Pivot, TEXT("Jog_Pivot"));
		if (bNeedWalkValidation)
		{
			CheckAnimGroup(Walk_Pivot, TEXT("Walk_Pivot"));
		}
		if (bNeedCrouchValidation)
		{
			CheckAnimGroup(Crouch_Pivot, TEXT("Crouch_Pivot"));
		}
	}

	if (bEnableSprintEntry) {
		CheckAnim(Sprint_Entry, TEXT("Sprint_Entry"));
	}
	if (bEnableSprintExit) {
		CheckAnim(Sprint_Exit, TEXT("Sprint_Exit"));
	}
	if (bNeedSprintValidation) {
		 CheckAnim(Sprint_Loop, TEXT("Sprint_Loop"));
	}
	
	if (bEnableFullJumpAnim) {
		CheckAnim(Jump_Start_Loop, TEXT("Jump_Start_Loop"));
		CheckAnim(Jump_Apex, TEXT("Jump_Apex"));
	}
	CheckAnim(Jump_Start, TEXT("Jump_Start"));
	CheckAnim(Jump_Fall_Loop, TEXT("Jump_Fall_Loop"));
	CheckAnim(Jump_Land, TEXT("Jump_Land"));
	
	if (bEnableLandAdditive) {
		CheckAnim(Jump_RecoveryAdditive, TEXT("Jump_RecoveryAdditive"));
	}

	if (bEnableAimOffset) {
		CheckAnim(Aim_HipFirePose, TEXT("Aim_HipFirePose"));
		CheckAnim(Aim_HipFirePose_Crouch, TEXT("Aim_HipFirePose_Crouch"));
		CheckAnim(IdleAimOffset_Relaxed, TEXT("IdleAimOffset_Relaxed"));
		CheckAnim(IdleAimOffset_ADS, TEXT("IdleAimOffset_ADS"));
	}
	
	if (bEnableAdditiveHurts) {
		const auto CheckAnimArrayGroup = [&](const FLAAnimSequenceArray4& AnimGroup, const FString& GroupName) {
			if (AnimGroup.F.Num() == 0) Context.AddError(FText::FromString(GroupName + TEXT(".F has no animations.")));
			if (AnimGroup.B.Num() == 0 && bCalcAdditiveHurtDirection) Context.AddError(FText::FromString(GroupName + TEXT(".B has no animations.")));
			if (AnimGroup.L.Num() == 0 && bCalcAdditiveHurtDirection) Context.AddError(FText::FromString(GroupName + TEXT(".L has no animations.")));
			if (AnimGroup.R.Num() == 0 && bCalcAdditiveHurtDirection) Context.AddError(FText::FromString(GroupName + TEXT(".R has no animations.")));
		};
		CheckAnimArrayGroup(AdditiveHurts, TEXT("AdditiveHurts"));
	}

	// --- Curve Validation ---
	if (bEnableRootYawOffset)
	{
		const auto CheckTurnInPlaceCurves = [&](UAnimSequence* AnimSequence, const FString& SlotName)
		{
			if (AnimSequence == nullptr) { return; }

			if (!UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence, TurnYawWeightCurveName, ERawCurveTrackTypes::RCT_Float))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::FromString(FString::Printf(TEXT("Animation '%s' in slot '%s' is missing curve '%s'."), *AnimSequence->GetName(), *SlotName, *TurnYawWeightCurveName.ToString())));
			}
			if (!UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence, RemainingTurnYawCurveName, ERawCurveTrackTypes::RCT_Float))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::FromString(FString::Printf(TEXT("Animation '%s' in slot '%s' is missing curve '%s'."), *AnimSequence->GetName(), *SlotName, *RemainingTurnYawCurveName.ToString())));
			}
		};
		CheckTurnInPlaceCurves(TurnInPlace_Left, TEXT("TurnInPlace_Left"));
		CheckTurnInPlaceCurves(TurnInPlace_Right, TEXT("TurnInPlace_Right"));
		if (bNeedCrouchValidation)
		{
			CheckTurnInPlaceCurves(Crouch_TurnInPlace_Left, TEXT("Crouch_TurnInPlace_Left"));
			CheckTurnInPlaceCurves(Crouch_TurnInPlace_Right, TEXT("Crouch_TurnInPlace_Right"));
		}
		
	}

	const auto CheckLocomotionGroup = [&](const FLAAnimSequence4& AnimGroup, const FString& GroupName)
	{
		const auto Check = [&](UAnimSequence* Anim, const FString& Slot)
		{
			if (Anim == nullptr) { return; }
			if (!UAnimationBlueprintLibrary::DoesCurveExist(Anim, LocomotionDistanceCurveName, ERawCurveTrackTypes::RCT_Float))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::FromString(FString::Printf(TEXT("Animation '%s' in slot '%s' is missing curve '%s'."), *Anim->GetName(), *Slot, *LocomotionDistanceCurveName.ToString())));
			}
		};
		Check(AnimGroup.F, GroupName + TEXT(".F"));
		Check(AnimGroup.B, GroupName + TEXT(".B"));
		Check(AnimGroup.L, GroupName + TEXT(".L"));
		Check(AnimGroup.R, GroupName + TEXT(".R"));
	};

	if (bEnableStartAnims)
	{
		CheckLocomotionGroup(Jog_Start, TEXT("Jog_Start"));
		if (bNeedWalkValidation)
		{
			CheckLocomotionGroup(Walk_Start, TEXT("Walk_Start"));
		}
		if (bNeedCrouchValidation)
		{
			CheckLocomotionGroup(Crouch_Start, TEXT("Crouch_Start"));
		}
	}
	if (bEnableStopAnims)
	{
		CheckLocomotionGroup(Jog_Stop, TEXT("Jog_Stop"));
		if (bNeedWalkValidation)
		{
			CheckLocomotionGroup(Walk_Stop, TEXT("Walk_Stop"));
		}
		if (bNeedCrouchValidation)
		{
			CheckLocomotionGroup(Crouch_Stop, TEXT("Crouch_Stop"));
		}
	}
	if (bEnablePivotAnim)
	{
		CheckLocomotionGroup(Jog_Pivot, TEXT("Jog_Pivot"));
		if (bNeedWalkValidation)
		{
			CheckLocomotionGroup(Walk_Pivot, TEXT("Walk_Pivot"));
		}
		if (bNeedCrouchValidation)
		{
			CheckLocomotionGroup(Crouch_Pivot, TEXT("Crouch_Pivot"));
		}
	}

	if (Jump_Land != nullptr && bNeedJumpValidation)
	{
		if (!UAnimationBlueprintLibrary::DoesCurveExist(Jump_Land, JumpDistanceCurveName, ERawCurveTrackTypes::RCT_Float))
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString(FString::Printf(TEXT("Animation '%s' in slot 'Jump_Land' is missing curve '%s'."), *Jump_Land->GetName(), *JumpDistanceCurveName.ToString())));
		}
	}

	return Result;
}


#endif
