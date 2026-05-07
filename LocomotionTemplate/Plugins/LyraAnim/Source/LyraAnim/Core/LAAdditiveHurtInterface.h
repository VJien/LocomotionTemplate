// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraAnim/Data/LATypes.h"
#include "UObject/Interface.h"
#include "LAAdditiveHurtInterface.generated.h"

/**
 * 受击叠加动画接口
 * Interface for triggering directional additive hurt animations.
 * Implemented by ULAAnimInstance to allow gameplay code to trigger
 * hit reactions from any context (abilities, damage handlers, etc.).
 */
UINTERFACE()
class ULAAdditiveHurtInterface : public UInterface
{
	GENERATED_BODY()
};

class LYRAANIM_API ILAAdditiveHurtInterface
{
	GENERATED_BODY()

public:
	/**
	 * 应用受击叠加动画
	 * Trigger an additive hurt animation in the given direction.
	 * The implementation should handle alternating between triggers
	 * to allow re-triggering without waiting for the previous hurt.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=AnimInterface)
	void ApplyAdditiveHurt(ELADirection4 Direction);

	/**
	 * 停止受击叠加动画
	 * Stop the currently playing additive hurt animation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=AnimInterface)
	void StopAdditiveHurt();
};
