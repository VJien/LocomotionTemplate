#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LTHUD.generated.h"

class SLTDebugPanel;
class SWidget;

UCLASS()
class LOCOMOTIONTEMPLATE_API ALTHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALTHUD();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugPanel();

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	FText DebugHintText;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	bool bShowDebugHint = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	FText ControlHints;

private:
	TSharedPtr<SLTDebugPanel> DebugPanel;
	TSharedPtr<SWidget> HintWidget;
	bool bDebugPanelVisible = false;

	void ShowDebugPanel();
	void HideDebugPanel();
	void ShowHintBar();
	void HideHintBar();
};
