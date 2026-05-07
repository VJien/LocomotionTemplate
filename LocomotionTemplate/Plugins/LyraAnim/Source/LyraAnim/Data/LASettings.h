#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LASettings.generated.h"

/**
 * LyraAnim 插件全局设置
 * Project-wide settings for the LyraAnim plugin.
 * Access via: Project Settings -> Game -> LA Settings
 */
UCLASS(config = LA, defaultconfig, meta = (DisplayName = "LA Settings"))
class LYRAANIM_API ULASettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULASettings();

	/** 获取设置单例 */
	static ULASettings* Get()
	{
		return GetMutableDefault<ULASettings>();
	}

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("LA Settings"); }

	/**
	 * 启用 DataAsset 数据验证
	 * When enabled, ULAAnimConfigDataAsset::IsDataValid() will check animation
	 * references and curve existence, reporting errors for missing/invalid data.
	 * Disable to skip validation (useful during rapid prototyping).
	 */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bEnableDataAssetValidation = true;

};
