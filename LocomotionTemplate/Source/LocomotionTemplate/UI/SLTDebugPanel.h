#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ULTDebugSubsystem;
class ULASettings;
class ALTCharacter;
class SSlider;
struct FSlateBrush;

class SLTDebugPanel : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLTDebugPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetCharacter(TWeakObjectPtr<ALTCharacter> InCharacter);

private:
	TWeakObjectPtr<ULTDebugSubsystem> DebugSubsystem;
	TWeakObjectPtr<ALTCharacter> CachedCharacter;

	TSharedPtr<SCheckBox> StartDMCheckBox;
	TSharedPtr<SCheckBox> StopDMCheckBox;
	TSharedPtr<SVerticalBox> StartDMRow;
	TSharedPtr<SVerticalBox> StopDMRow;
	TSharedPtr<SVerticalBox> PivotSWRow;
	TArray<TSharedPtr<FSlateBrush>> CharacterSetIconBrushes;

	TSharedRef<SWidget> BuildSliderRow(
		const FText& Label,
		float MinValue, float MaxValue,
		TAttribute<float> ValueAttr,
		TFunction<void(float)> OnChanged);

	TSharedRef<SWidget> BuildToggleRow(
		const FText& Label,
		FName PropertyName,
		TAttribute<bool> IsEnabledAttr = TAttribute<bool>(true));

	TSharedRef<SWidget> BuildConditionalToggleRow(
		const FText& Label,
		FName PropertyName,
		TSharedPtr<SVerticalBox>& RowSlot,
		TAttribute<bool> IsEnabledAttr = TAttribute<bool>(true));

	TSharedRef<SWidget> BuildStatusSection();
	TSharedRef<SWidget> BuildCharacterSetSection();
	TSharedRef<SWidget> BuildCharacterSetButtonContent(int32 Index);

	ECheckBoxState IsToggleChecked(FName PropertyName) const;
	void OnToggleCheckChanged(FName PropertyName, ECheckBoxState NewState);
	FReply OnCharacterSetClicked(int32 Index);
	FSlateColor GetCharacterSetButtonColor(int32 Index) const;
	FText GetCharacterSetDisplayName(int32 Index) const;

	bool IsBlendSpaceMode() const;
	bool IsStartEnabled() const;
	bool IsStopEnabled() const;
	bool IsPivotEnabled() const;
	bool IsAOEnabled() const;

	void UpdateConditionalVisibility();

	FText GetSpeedText() const;
	FText GetMovementStateText() const;
	FText GetStrafeText() const;
	FSlateColor GetStatusColor() const;
};
