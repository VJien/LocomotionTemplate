// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimPose.h"
#include "DistanceCurveModifier.h"
#include "FootstepAnimEventsModifier.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LABlueprintLibrary.generated.h"


USTRUCT(BlueprintType)
struct FLAFootDefinition
{
	GENERATED_BODY()
	FFootDefinition ToFootDefinition() const
	{
		FFootDefinition FootDef;
		FootDef.FootBoneName = FootBoneName;
		FootDef.ReferenceBoneName = ReferenceBoneName;
		FootDef.bShouldGenerateSyncMarkers = bShouldGenerateSyncMarkers;
		FootDef.SyncMarkerTrackName = SyncMarkerTrackName;
		FootDef.SyncMarkerName = SyncMarkerName;
		FootDef.SyncMarkerDetectionTechnique = SyncMarkerDetectionTechnique;
		FootDef.bShouldGenerateNotifies = bShouldGenerateNotifies;
		FootDef.FootstepNotifyTrackName = FootstepNotifyTrackName;
		FootDef.FootstepNotify = FootstepNotify;
		FootDef.FootstepNotifyDetectionTechnique = FootstepNotifyDetectionTechnique;
		
		return FootDef;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FName FootBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FName ReferenceBoneName = FName(TEXT("root"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync Markers")
	bool bShouldGenerateSyncMarkers = false; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync Markers", meta=(EditCondition="bShouldGenerateSyncMarkers"))
	FName SyncMarkerTrackName = TEXT("FootSyncMarkers");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync Markers", meta=(EditCondition="bShouldGenerateSyncMarkers"))
	FName SyncMarkerName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync Markers", meta=(EditCondition="bShouldGenerateSyncMarkers"))
	EDetectionTechnique SyncMarkerDetectionTechnique = EDetectionTechnique::PassThroughReferenceBone;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notifies")
	bool bShouldGenerateNotifies = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notifies", meta=(EditCondition="bShouldGenerateNotifies"))
	FName FootstepNotifyTrackName = TEXT("FootAnimEvents");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notifies", meta=(EditCondition="bShouldGenerateNotifies"))
	TSubclassOf<UAnimNotify> FootstepNotify = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notifies", meta=(EditCondition="bShouldGenerateNotifies"))
	EDetectionTechnique FootstepNotifyDetectionTechnique = EDetectionTechnique::FootBoneReachesGround;
};

UCLASS()
class ULABlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	struct FFootSampleState
	{
		double GroundLevel = UE_DOUBLE_BIG_NUMBER;
		double RefBoneTranslationDotRefBoneToFootBoneVec = 0.0;
		double PrevRefBoneTranslationDotRefBoneToFootBoneVec = 0.0;
		bool bIsFootBoneInGround = false;
		bool bWasFootBoneInGround = false; 
		float FootBoneSpeed = 0.0f;
		float PrevFootBoneSpeed = 0.0f;
		float MaxFootSpeed = -MAX_FLT;
		float MinFootSpeedBelowThreshold = MAX_FLT;
		float TimeAtMinFootSpeedBelowThreshold = MAX_FLT;
	};
protected:
	static float CalculateMagnitude(const FVector& Vector, EDistanceCurve_Axis Axis);
	static float CalculateMagnitudeSq(const FVector& Vector, EDistanceCurve_Axis Axis);
	static void ValidateNotifyTracks(UAnimSequence* InAnimation, const TArray<FFootDefinition>& FootDefinitions,  bool bShouldRemovePreExistingNotifiesOrSyncMarkers);
	static void GatherNotifyTracksInfo(const UAnimSequence* InAnimation, const TArray<FFootDefinition>& FootDefinitions);
	static void PrepareNotifyTrack(UAnimSequence* InAnimation, FName InRequestedNotifyTrackName, bool bShouldRemovePreExistingNotifiesOrSyncMarkers);
	static bool CanWePlaceEventAtSample(const FFootSampleState & InTestLegSampleState, EDetectionTechnique DetectionTechnique,float SpeedThreshold);
	static float ComputeBoneSpeed(const FAnimPose& InPose, const FAnimPose &InFuturePose, float InDelta, FName InFootBoneName);

	static int32 PopulateCurveKeys(UAnimSequence* AnimSequence,FName RootBoneNameName = "root",FName TurnYawCurveName = "RemainingTurnYaw");
public:
	UFUNCTION(BlueprintCallable,Category= "Animation|LABlueprintLibrary")
	static void AddDistanceCurve(UAnimSequence* AnimSequence,FName CurveName = "Distance",int32 SampleRate = 30,float StopSpeedThreshold = 5.0f,
		EDistanceCurve_Axis Axis = EDistanceCurve_Axis::XY,bool bStopAtEnd = false,bool bLockRootMotionWhenFinished = true);
	UFUNCTION(BlueprintCallable,Category= "Animation|LABlueprintLibrary")
	static void AddFootstepNotify(UAnimSequence* AnimSequence, int32 SampleRate, float GroundThreshold,float SpeedThreshold,const TArray<FLAFootDefinition>& FootDefinitions,bool bShouldRemovePreExistingNotifiesOrSyncMarkers);
	UFUNCTION(BlueprintCallable,Category= "Animation|LABlueprintLibrary")
	static void AddTurnYawCurve(UAnimSequence* AnimSequence,FName RootBoneName = "root",FName TurnYawCurveName = "RemainingTurnYaw", FName WeightCurveName = "TurnYawWeight");
	UFUNCTION(BlueprintCallable,Category= "Animation|LABlueprintLibrary")
	static void AddTurnYawWeightCurveFromTurnYawCurve(UAnimSequence* AnimSequence,FName TurnYawCurveName = "RemainingTurnYaw", FName WeightCurveName = "TurnYawWeight");
	UFUNCTION(BlueprintCallable,Category= "Animation|LABlueprintLibrary")
	static void ModifyCurveValues(UAnimSequence* AnimSequence,FName CurveName,float Scale = 1.0f,float Offset = 0.0f,bool bClampMin = false,float MinValue = 0.0f,bool bClampMax = false,float MaxValue = 1.0f);

};
