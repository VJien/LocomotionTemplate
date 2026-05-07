// Copyright Epic Games, Inc. All Rights Reserved.

#include "LABlueprintLibrary.h"

#include "AnimPose.h"

ULABlueprintLibrary::ULABlueprintLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float ULABlueprintLibrary::CalculateMagnitude(const FVector& Vector, EDistanceCurve_Axis Axis)
{
	switch (Axis)
	{
	case EDistanceCurve_Axis::X:		return FMath::Abs(Vector.X); break;
	case EDistanceCurve_Axis::Y:		return FMath::Abs(Vector.Y); break;
	case EDistanceCurve_Axis::Z:		return FMath::Abs(Vector.Z); break;
	default: return FMath::Sqrt(CalculateMagnitudeSq(Vector, Axis)); break;
	}
}

float ULABlueprintLibrary::CalculateMagnitudeSq(const FVector& Vector, EDistanceCurve_Axis Axis)
{
	switch (Axis)
	{
	case EDistanceCurve_Axis::X:		return FMath::Square(FMath::Abs(Vector.X)); break;
	case EDistanceCurve_Axis::Y:		return FMath::Square(FMath::Abs(Vector.Y)); break;
	case EDistanceCurve_Axis::Z:		return FMath::Square(FMath::Abs(Vector.Z)); break;
	case EDistanceCurve_Axis::XY:		return Vector.X * Vector.X + Vector.Y * Vector.Y; break;
	case EDistanceCurve_Axis::XZ:		return Vector.X * Vector.X + Vector.Z * Vector.Z; break;
	case EDistanceCurve_Axis::YZ:		return Vector.Y * Vector.Y + Vector.Z * Vector.Z; break;
	case EDistanceCurve_Axis::XYZ:		return Vector.X * Vector.X + Vector.Y * Vector.Y + Vector.Z * Vector.Z; break;
	default: check(false); break;
	}

	return 0.f;
}

void ULABlueprintLibrary::ValidateNotifyTracks(UAnimSequence* InAnimation,
	const TArray<FFootDefinition>& FootDefinitions, bool bShouldRemovePreExistingNotifiesOrSyncMarkers)
{
	check(InAnimation != nullptr)

	GatherNotifyTracksInfo(InAnimation,FootDefinitions);
	
	for (const FFootDefinition & FootDef : FootDefinitions)
	{
		if (FootDef.bShouldGenerateSyncMarkers)
		{
			PrepareNotifyTrack(InAnimation, FootDef.SyncMarkerTrackName,bShouldRemovePreExistingNotifiesOrSyncMarkers);
		}
		
		if (FootDef.bShouldGenerateNotifies)
		{
			PrepareNotifyTrack(InAnimation, FootDef.FootstepNotifyTrackName,bShouldRemovePreExistingNotifiesOrSyncMarkers);
		}
	}
}

void ULABlueprintLibrary::GatherNotifyTracksInfo(const UAnimSequence* InAnimation,
	const TArray<FFootDefinition>& FootDefinitions)
{
	for (const FFootDefinition& FootDef : FootDefinitions)
	{
		// Determine tracks that will be generated and/or processed
		const bool bDoesRequestedSyncTrackAlreadyExist = UAnimationBlueprintLibrary::IsValidAnimNotifyTrackName(InAnimation, FootDef.SyncMarkerTrackName);
		const bool bDoesRequestedNotifyTrackAlreadyExist = UAnimationBlueprintLibrary::IsValidAnimNotifyTrackName(InAnimation, FootDef.FootstepNotifyTrackName);

		if (FootDef.bShouldGenerateSyncMarkers)
		{
			if (!bDoesRequestedSyncTrackAlreadyExist)
			{
				//GeneratedNotifyTracks.Add(FootDef.SyncMarkerTrackName);
			}

			//ProcessedNotifyTracks.Add(FootDef.SyncMarkerTrackName);
		}

		if (FootDef.bShouldGenerateNotifies)
		{
			if (!bDoesRequestedNotifyTrackAlreadyExist)
			{
				//GeneratedNotifyTracks.Add(FootDef.FootstepNotifyTrackName);
			}

			//ProcessedNotifyTracks.Add(FootDef.FootstepNotifyTrackName);
		}
	}
}

void ULABlueprintLibrary::PrepareNotifyTrack(UAnimSequence* InAnimation, FName InRequestedNotifyTrackName,
	bool bShouldRemovePreExistingNotifiesOrSyncMarkers)
{
		const bool bDoesTrackNameAlreadyExist = UAnimationBlueprintLibrary::IsValidAnimNotifyTrackName(InAnimation, InRequestedNotifyTrackName);
	
		if (!bDoesTrackNameAlreadyExist)
		{
			UAnimationBlueprintLibrary::AddAnimationNotifyTrack(InAnimation, InRequestedNotifyTrackName, FLinearColor::MakeRandomColor());
		}
		else if (bShouldRemovePreExistingNotifiesOrSyncMarkers)
		{
			UAnimationBlueprintLibrary::RemoveAnimationNotifyEventsByTrack(InAnimation, InRequestedNotifyTrackName);
			UAnimationBlueprintLibrary::RemoveAnimationSyncMarkersByTrack(InAnimation, InRequestedNotifyTrackName);
		}
}

bool ULABlueprintLibrary::CanWePlaceEventAtSample(const FFootSampleState& InFootSampleState,
	EDetectionTechnique DetectionTechnique, float SpeedThreshold)
{
	switch(DetectionTechnique)
	{
	case EDetectionTechnique::PassThroughReferenceBone: return InFootSampleState.RefBoneTranslationDotRefBoneToFootBoneVec > 0.0f && InFootSampleState.PrevRefBoneTranslationDotRefBoneToFootBoneVec < 0.0f;
	case EDetectionTechnique::FootBoneReachesGround: return !InFootSampleState.bWasFootBoneInGround && InFootSampleState.bIsFootBoneInGround;
	case EDetectionTechnique::FootBoneSpeed: return InFootSampleState.PrevFootBoneSpeed < SpeedThreshold && InFootSampleState.FootBoneSpeed >= SpeedThreshold;
	default: return false;
	}
}

float ULABlueprintLibrary::ComputeBoneSpeed(const FAnimPose& InPose, const FAnimPose& InFuturePose, float InDelta,
	FName InFootBoneName)
{
	check(!FMath::IsNearlyZero(InDelta))
			
const FTransform FootBoneTransform = UAnimPoseExtensions::GetBonePose(InPose, InFootBoneName, EAnimPoseSpaces::World);
	const FTransform FutureFootBoneTransform = UAnimPoseExtensions::GetBonePose(InFuturePose, InFootBoneName, EAnimPoseSpaces::World);
	const double FootBoneDistance = (FutureFootBoneTransform.GetLocation() - FootBoneTransform.GetLocation()).Length();
			
	return static_cast<float>(FootBoneDistance / InDelta);
}

int32 ULABlueprintLibrary::PopulateCurveKeys(UAnimSequence* AnimSequence,FName RootBoneNameName,FName TurnYawCurveName)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("PopulateCurveKeys failed. Reason: Invalid Animation"));
		return INDEX_NONE;
	}
	int32 NumFrames = 0;
	UAnimationBlueprintLibrary::GetNumFrames(AnimSequence,NumFrames);
	if (NumFrames <= 0)
	{
		UE_LOG(LogAnimation, Error, TEXT("PopulateCurveKeys failed. Reason: Animation has no frames"));
		return INDEX_NONE;
	}
	FAnimPose Pose;
	UAnimPoseExtensions::GetAnimPoseAtFrame(AnimSequence,NumFrames,FAnimPoseEvaluationOptions(),Pose);
	float TotalTurnYaw = UAnimPoseExtensions::GetBonePose(Pose,RootBoneNameName,EAnimPoseSpaces::Local).Rotator().Yaw;
	FAnimPoseEvaluationOptions EvalOptions { EAnimDataEvalType::Raw, true, false, true, nullptr, true, true };
	int32 FirstZeroFrame = INDEX_NONE;
	for (int32 i = 0; i<=NumFrames;++i)
	{
		FAnimPose FramePose;
		UAnimPoseExtensions::GetAnimPoseAtFrame(AnimSequence,i,EvalOptions,FramePose);
		float Yaw = UAnimPoseExtensions::GetBonePose(Pose,RootBoneNameName,EAnimPoseSpaces::Local).Rotator().Yaw;
		float Value = TotalTurnYaw - Yaw;
		UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,TurnYawCurveName,AnimSequence->GetTimeAtFrame(i),Value);
		if (FMath::IsNearlyZero(Yaw) && FirstZeroFrame == INDEX_NONE)
		{
			FirstZeroFrame = i;
		}
	}

	return FirstZeroFrame;
	
}

void ULABlueprintLibrary::AddDistanceCurve(UAnimSequence* Animation, FName CurveName, int32 SampleRate,
                                           float StopSpeedThreshold, EDistanceCurve_Axis Axis, bool bStopAtEnd, bool bLockRootMotionWhenFinished)
{
	if (Animation == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("DistanceCurveModifier failed. Reason: Invalid Animation"));
		return;
	}

	if (!Animation->HasRootMotion())
	{
		Animation->bEnableRootMotion = true;
	}
	Animation->bForceRootLock = false;
	
	const bool bMetaDataCurve = false;
	if (UAnimationBlueprintLibrary::DoesCurveExist(Animation,CurveName,ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(Animation, CurveName);
	}
	UAnimationBlueprintLibrary::AddCurve(Animation, CurveName, ERawCurveTrackTypes::RCT_Float, bMetaDataCurve);

	const float AnimLength = Animation->GetPlayLength();
	float SampleInterval;
	int32 NumSteps;
	float TimeOfMinSpeed;

	if(bStopAtEnd)
	{ 
		TimeOfMinSpeed = AnimLength;
	}
	else
	{
		// Perform a high resolution search to find the sample point with minimum speed.
		
		TimeOfMinSpeed = 0.f;
		float MinSpeedSq = FMath::Square(StopSpeedThreshold);

		SampleInterval = 1.f / 120.f;
		NumSteps = AnimLength / SampleInterval;
		for (int32 Step = 0; Step < NumSteps; ++Step)
		{
			const float Time = Step * SampleInterval;

			const bool bAllowLooping = false;
			FAnimExtractContext Context(Time,true,FDeltaTimeRecord(SampleInterval),bAllowLooping);
			const FVector RootMotionTranslation = Animation->ExtractRootMotion(Context).GetTranslation();
			const float RootMotionSpeedSq = CalculateMagnitudeSq(RootMotionTranslation, Axis) / SampleInterval;

			if (RootMotionSpeedSq < MinSpeedSq)
			{
				MinSpeedSq = RootMotionSpeedSq;
				TimeOfMinSpeed = Time;
			}
		}
	}

	SampleInterval = 1.f / SampleRate;
	NumSteps = FMath::CeilToInt(AnimLength / SampleInterval);
	float Time = 0.0f;
	for (int32 Step = 0; Step <= NumSteps && Time < AnimLength; ++Step)
	{
		Time = FMath::Min(Step * SampleInterval, AnimLength);

		// Assume that during any time before the stop/pivot point, the animation is approaching that point.
		// TODO: This works for clips that are broken into starts/stops/pivots, but needs to be rethought for more complex clips.
		const float ValueSign = (Time < TimeOfMinSpeed) ? -1.0f : 1.0f;
		FAnimExtractContext Context(TimeOfMinSpeed,true,FDeltaTimeRecord(SampleInterval));
		const FVector RootMotionTranslation = Animation->ExtractRootMotionFromRange(TimeOfMinSpeed, Time,Context).GetTranslation();
		UAnimationBlueprintLibrary::AddFloatCurveKey(Animation, CurveName, Time, ValueSign * CalculateMagnitude(RootMotionTranslation, Axis));
	}
	if (bLockRootMotionWhenFinished)
	{
		Animation->bForceRootLock = true;
	}
}

void ULABlueprintLibrary::AddFootstepNotify(UAnimSequence* InAnimation, int32 SampleRate, float GroundThreshold,
	float SpeedThreshold, const TArray<FLAFootDefinition>& FootDefinitions,
	bool bShouldRemovePreExistingNotifiesOrSyncMarkers)
{
	if (InAnimation == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("FootstepAnimEventsModifierBase failed. Reason: Invalid Animation"));
		return;
	}
	TArray<FFootDefinition> CommonDefinitions;
	for (auto&& Df:FootDefinitions)
	{
		CommonDefinitions.Add(Df.ToFootDefinition());
	}
	// Disable root motion lock during application
	TGuardValue<bool> ForceRootLockGuard(InAnimation->bForceRootLock, false);
	
	// Clean up or generate tracks if needed
	ValidateNotifyTracks(InAnimation, CommonDefinitions,bShouldRemovePreExistingNotifiesOrSyncMarkers);
	
	// Process animation asset
	{
		const FAnimPoseEvaluationOptions AnimPoseEvalOptions { EAnimDataEvalType::Raw, true, false, false, nullptr, true, false };
		const float SequenceLength = InAnimation->GetPlayLength();
		const float SampleStep = 1.0f / static_cast<float>(SampleRate);
		const int SampleNum = FMath::TruncToInt(SequenceLength / SampleStep);
		
		TArray<FFootSampleState> FootSampleStates;
		FootSampleStates.Init(FFootSampleState(), FootDefinitions.Num());
		
		// Get ground levels and max speed values.
		for (int SampleIndex = 0; SampleIndex < SampleNum; ++SampleIndex)
		{
			const float SampleTime = FMath::Clamp(static_cast<float>(SampleIndex) * SampleStep, 0.0f, SequenceLength);
			const float FutureSampleTime = FMath::Clamp((static_cast<float>(SampleIndex) + 1.0f) * SampleStep, 0.0f, SequenceLength);

			FAnimPose AnimPose;
			FAnimPose FutureAnimPose;
			
			UAnimPoseExtensions::GetAnimPoseAtTime(InAnimation, SampleTime, AnimPoseEvalOptions, AnimPose);
			UAnimPoseExtensions::GetAnimPoseAtTime(InAnimation, FutureSampleTime, AnimPoseEvalOptions, FutureAnimPose);
			
			for (int FootIndex = 0; FootIndex < FootDefinitions.Num(); ++FootIndex)
			{
				const FFootDefinition & FootDef = FootDefinitions[FootIndex].ToFootDefinition();
				FFootSampleState & FootState = FootSampleStates[FootIndex];
				
				FootState.GroundLevel = FMath::Min(FootState.GroundLevel, UAnimPoseExtensions::GetBonePose(AnimPose, FootDef.FootBoneName, EAnimPoseSpaces::World).GetLocation().Z);
				FootState.MaxFootSpeed = FMath::Max(FootState.MaxFootSpeed, FMath::Abs(ComputeBoneSpeed(AnimPose, FutureAnimPose, SampleStep, FootDef.FootBoneName)));
			}
		}

		// Generate animation events
		for (int SampleIndex = 0; SampleIndex < (SampleNum - 1); ++SampleIndex)
		{
			const float SampleTime = FMath::Clamp(static_cast<float>(SampleIndex) * SampleStep, 0.0f, SequenceLength);
			const float FutureSampleTime = FMath::Clamp((static_cast<float>(SampleIndex) + 1.0f) * SampleStep, 0.0f, SequenceLength);

			FAnimPose AnimPose;
			FAnimPose FutureAnimPose;
			
			// Query animation pose at the current sample time
			UAnimPoseExtensions::GetAnimPoseAtTime(InAnimation, SampleTime, AnimPoseEvalOptions, AnimPose);
			UAnimPoseExtensions::GetAnimPoseAtTime(InAnimation, FutureSampleTime, AnimPoseEvalOptions, FutureAnimPose);
			
			// Process all foot definitions
			for (int CurrFootIdx = 0; CurrFootIdx < FootDefinitions.Num(); ++CurrFootIdx)
			{
				const FFootDefinition & FootDef = FootDefinitions[CurrFootIdx].ToFootDefinition();
				FFootSampleState & FootState = FootSampleStates[CurrFootIdx];
				
				// Update current sample info
				{
					const FTransform FootBoneTransform = UAnimPoseExtensions::GetBonePose(AnimPose, FootDef.FootBoneName, EAnimPoseSpaces::World);

					// Method: PassThroughReferenceBone
					{
						// Get reference bone translation
						const FTransform ReferenceBoneTransform = UAnimPoseExtensions::GetBonePose(AnimPose, FootDef.ReferenceBoneName, EAnimPoseSpaces::World);
						const FTransform FutureReferenceBoneTransform = UAnimPoseExtensions::GetBonePose(FutureAnimPose, FootDef.ReferenceBoneName, EAnimPoseSpaces::World);
						const FVector ReferenceBoneTranslation = FutureReferenceBoneTransform.GetLocation() - ReferenceBoneTransform.GetLocation();

						// Get foot bone direction with respect to reference bone
						const FVector ReferenceBoneToFootBoneVector = FootBoneTransform.GetLocation() - ReferenceBoneTransform.GetLocation();
					
						FootState.RefBoneTranslationDotRefBoneToFootBoneVec = FVector::DotProduct(ReferenceBoneTranslation, ReferenceBoneToFootBoneVector);
					}

					// Method: FootBoneReachesGround
					FootState.bIsFootBoneInGround = FMath::Abs(FootState.GroundLevel - FootBoneTransform.GetLocation().Z) <= GroundThreshold;

					// Method: FootBoneSpeed
					FootState.FootBoneSpeed = FMath::Abs(ComputeBoneSpeed(AnimPose, FutureAnimPose, SampleStep, FootDef.FootBoneName)) / FootState.MaxFootSpeed;

					// Keep track of sample with lowest speed below threshold
					if (FootState.FootBoneSpeed < SpeedThreshold)
					{
						if (FootState.FootBoneSpeed < FootState.MinFootSpeedBelowThreshold)
						{
							FootState.TimeAtMinFootSpeedBelowThreshold = SampleTime;
							FootState.MinFootSpeedBelowThreshold = FootState.FootBoneSpeed;
						}
					}
				}
				
				if (SampleIndex > 0)
				{
					// Generate sync markers
					if (FootDef.bShouldGenerateSyncMarkers && CanWePlaceEventAtSample(FootState, FootDef.SyncMarkerDetectionTechnique,SpeedThreshold))
					{
						float FinalSyncMarkerTime = FootDef.SyncMarkerDetectionTechnique == EDetectionTechnique::FootBoneSpeed ? FootState.TimeAtMinFootSpeedBelowThreshold : SampleTime;
						UAnimationBlueprintLibrary::AddAnimationSyncMarker(InAnimation, FootDef.SyncMarkerName, FinalSyncMarkerTime, FootDef.SyncMarkerTrackName);
					}

					// Generate foot step fx notifies
					if (FootDef.bShouldGenerateNotifies && CanWePlaceEventAtSample(FootState, FootDef.FootstepNotifyDetectionTechnique,SpeedThreshold))
					{
						float FinalAnimNotifyTime = FootDef.FootstepNotifyDetectionTechnique == EDetectionTechnique::FootBoneSpeed ? FootState.TimeAtMinFootSpeedBelowThreshold : SampleTime;
						UAnimationBlueprintLibrary::AddAnimationNotifyEvent(InAnimation, FootDef.FootstepNotifyTrackName, FinalAnimNotifyTime, FootDef.FootstepNotify);
					}
				}
				
				// Update foot state
				{
					const bool bDidWePlaceAnyEventsUsingFootBoneSpeed = CanWePlaceEventAtSample(FootState, EDetectionTechnique::FootBoneSpeed,SpeedThreshold);

					// Reset minimum values if we are above speed threshold
					if (bDidWePlaceAnyEventsUsingFootBoneSpeed)
					{
						FootState.TimeAtMinFootSpeedBelowThreshold = MAX_FLT;
						FootState.MinFootSpeedBelowThreshold = MAX_FLT;
					}
					
					// Keep track of foot info
					FootState.PrevFootBoneSpeed = FootState.FootBoneSpeed;
					FootState.bWasFootBoneInGround = FootState.bIsFootBoneInGround;
					FootState.PrevRefBoneTranslationDotRefBoneToFootBoneVec = FootState.RefBoneTranslationDotRefBoneToFootBoneVec;
				}
			}
		}
	}
}

void ULABlueprintLibrary::AddTurnYawCurve(UAnimSequence* AnimSequence, FName RootBoneNameCurveName,
	FName TurnYawCurveName, FName WeightCurveName)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("TurnYawCurveModifier failed. Reason: Invalid Animation"));
		return;
	}
	if (UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence,TurnYawCurveName,ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimSequence, TurnYawCurveName);
	}
	UAnimationBlueprintLibrary::AddCurve(AnimSequence, TurnYawCurveName, ERawCurveTrackTypes::RCT_Float, false);
	int32 FirstZeroFrame = PopulateCurveKeys(AnimSequence,RootBoneNameCurveName,TurnYawCurveName);
	if (FirstZeroFrame == INDEX_NONE)
	{
		UE_LOG(LogAnimation, Error, TEXT("TurnYawCurveModifier failed. Reason: Animation does not turn"));
		return;
	}
	UAnimationBlueprintLibrary::AddCurve(AnimSequence, WeightCurveName, ERawCurveTrackTypes::RCT_Float, false);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,0.f,1.0f);
	float Time;
	UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,FirstZeroFrame,Time);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,Time,1.f);
	UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,FirstZeroFrame+1,Time);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,Time,0.f);
}

void ULABlueprintLibrary::AddTurnYawWeightCurveFromTurnYawCurve(UAnimSequence* AnimSequence, FName TurnYawCurveName,
	FName WeightCurveName)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("TurnYawWeightCurveFromTurnYawCurve failed. Reason: Invalid Animation"));
		return;
	}
	if (!UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence,TurnYawCurveName,ERawCurveTrackTypes::RCT_Float))
	{
		UE_LOG(LogAnimation, Error, TEXT("TurnYawWeightCurveFromTurnYawCurve failed. Reason: Turn Yaw Curve does not exist"));
		return;
	}
	//查询TurnYawCurveName曲线的关键帧，在第一个到0的帧添加1.0关键帧，在下一帧添加0.0关键帧
	int32 NumFrames = 0;
	UAnimationBlueprintLibrary::GetNumFrames(AnimSequence,NumFrames);
	if (NumFrames <= 0)
	{
		UE_LOG(LogAnimation, Error, TEXT("PopulateCurveKeys failed. Reason: Animation has no frames"));
		return;
	}
	int32 FirstZeroFrame = INDEX_NONE;
	for (int32 i = 0; i<NumFrames;++i)
	{
		float Value = 0.f;
		float Time = 0.f;
		UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,i,Time);
		Value = UAnimationBlueprintLibrary::GetFloatValueAtTime(AnimSequence,TurnYawCurveName,Time);
		if (FMath::IsNearlyZero(Value) && FirstZeroFrame == INDEX_NONE)
		{
			FirstZeroFrame = i;
			break;
		}
	}
	if (FirstZeroFrame == INDEX_NONE || FirstZeroFrame == NumFrames -1)
	{
		UE_LOG(LogAnimation, Error, TEXT("TurnYawWeightCurveFromTurnYawCurve failed. Reason: Animation does not turn"));
		return;
	}
	if (UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence,WeightCurveName,ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimSequence, WeightCurveName);
	}
	UAnimationBlueprintLibrary::AddCurve(AnimSequence, WeightCurveName, ERawCurveTrackTypes::RCT_Float, false);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,0.f,1.0f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,0.f,1.0f);
	float Time;
	UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,FirstZeroFrame,Time);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,Time,1.f);
	UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,FirstZeroFrame+1,Time);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimSequence,WeightCurveName,Time,0.f);

}

void ULABlueprintLibrary::ModifyCurveValues(UAnimSequence* AnimSequence, FName CurveName, float Scale, float Offset,
	bool bClampMin, float MinValue, bool bClampMax, float MaxValue)
{
	if (AnimSequence == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("ModifyCurveValues failed. Reason: Invalid Animation"));
		return;
	}
	if (!UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence,CurveName,ERawCurveTrackTypes::RCT_Float))
	{
		UE_LOG(LogAnimation, Error, TEXT("ModifyCurveValues failed. Reason: Curve does not exist"));
		return;
	}
	int32 NumFrames = 0;
	UAnimationBlueprintLibrary::GetNumFrames(AnimSequence,NumFrames);
	TArray<float> Times;
	TArray<float> Values;
	for (int32 i = 0; i<NumFrames;++i)
	{
		float Value = 0.f;
		float Time = 0.f;
		UAnimationBlueprintLibrary::GetTimeAtFrame(AnimSequence,i,Time);
		Value = UAnimationBlueprintLibrary::GetFloatValueAtTime(AnimSequence,CurveName,Time);
		Value = Value * Scale + Offset;
		if (bClampMin)
		{
			Value = FMath::Max(Value,MinValue);
		}
		if (bClampMax)
		{
			Value = FMath::Min(Value,MaxValue);
		}
		Times.Add(Time);
		Values.Add(Value);
	}
	
	UAnimationBlueprintLibrary::RemoveCurve(AnimSequence, CurveName);
	UAnimationBlueprintLibrary::AddCurve(AnimSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);
	UAnimationBlueprintLibrary::AddFloatCurveKeys(AnimSequence,CurveName,Times,Values);
}
