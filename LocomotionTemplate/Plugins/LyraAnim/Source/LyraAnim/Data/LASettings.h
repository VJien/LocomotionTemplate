#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LASettings.generated.h"

USTRUCT(BlueprintType)
struct FLAAnimToggleSettings
{
	GENERATED_BODY()

	//是否 使用BlendSpace，默认关闭，开启以后关闭下列几乎所有选项
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bUseBlendSpaceLoop = false;
	//开启Start动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableStart = true;
	//开启Start动画的DistanceMatch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnableStart && !bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableStartDM = true;
	//开启Start动画的StrideWarping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnableStart && !bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableStartStrideWarping = true;	
	 //开启Stop动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableStop = true;
   // 开启Cycle动画的DistanceMatch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnableStop && !bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableStopDM = true;
	 // 开启Stop动画的StrideWarping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnableStop && !bUseBlendSpaceLoop", EditConditionHides))
   bool bEnableStopStrideWarping = true;
	// 开启Cycle动画的DistanceMatch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableCycleDM = true;
	// 开启Cycle动画的StrideWarping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
   bool bEnableCycleStrideWarping = true;
	// 开启Pivot动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnablePivot = true;
	// 开启Pivot动画的StrideWarping. Pivot必须开启DM，不开配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnablePivot && !bUseBlendSpaceLoop", EditConditionHides))
	bool bEnablePivotStrideWarping= true;
	// 开启AimOffset动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableAimOffset = true;
	// 开启TurnInPlace动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "bEnableAimOffset && !bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableTurnInPlace = true;
	// 开启FootIK动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableFootIK = true;
	// 开启Lean动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (EditCondition = "!bUseBlendSpaceLoop", EditConditionHides))
	bool bEnableLean = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLAAnimToggleChanged, FName, PropertyName, bool, bNewValue);

UCLASS(config = LA, defaultconfig, meta = (DisplayName = "LA Settings"))
class LYRAANIM_API ULASettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULASettings();

	static ULASettings* Get()
	{
		return GetMutableDefault<ULASettings>();
	}

	static const ULASettings* GetConst()
	{
		return GetDefault<ULASettings>();
	}

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("LA Settings"); }

	const FLAAnimToggleSettings& GetToggleSettings() const { return ToggleSettings; }
	void SetToggleSettings(const FLAAnimToggleSettings& InSettings);

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bEnableDataAssetValidation = true;

	//全局开关，默认开启，与DA中的部分开关共同作用
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FLAAnimToggleSettings ToggleSettings;

	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FLAAnimToggleChanged OnToggleChanged;

private:
	void DiffAndBroadcast(const FLAAnimToggleSettings& Old, const FLAAnimToggleSettings& New);
};
