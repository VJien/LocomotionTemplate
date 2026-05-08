#include "UI/SLTDebugPanel.h"
#include "Core/LTDebugSubsystem.h"
#include "Core/LTProjectSettings.h"
#include "Character/LTCharacter.h"
#include "Data/LTCharacterSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/SListView.h"
#include "Styling/SlateBrush.h"
#include "LyraAnim/Data/LASettings.h"

#define LOCTEXT_NAMESPACE "SLTDebugPanel"

void SLTDebugPanel::Construct(const FArguments& InArgs)
{
	const ULTProjectSettings* PS = ULTProjectSettings::Get();

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.0f)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "Locomotion 调试面板"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FLinearColor::White)
			]

			// --- CMC Sliders ---
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CMCHeader", "角色运动参数"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildSliderRow(
					LOCTEXT("Accel", "加速度"),
					PS ? PS->MaxAccelerationRange.Min : 0.f,
					PS ? PS->MaxAccelerationRange.Max : 4096.f,
					TAttribute<float>::CreateLambda([this]() -> float
					{
						if (!DebugSubsystem.IsValid()) return 0.f;
						return DebugSubsystem->GetCMCParams().MaxAcceleration;
					}),
					[this](float Val)
					{
						if (!DebugSubsystem.IsValid()) return;
						FLTCMCParams P = DebugSubsystem->GetCMCParams();
						P.MaxAcceleration = Val;
						DebugSubsystem->SetCMCParams(P);
						if (CachedCharacter.IsValid())
						{
							CachedCharacter->GetCharacterMovement()->MaxAcceleration = Val;
						}
					})
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildSliderRow(
					LOCTEXT("Brake", "制动减速度"),
					PS ? PS->BrakingDecelerationRange.Min : 0.f,
					PS ? PS->BrakingDecelerationRange.Max : 4096.f,
					TAttribute<float>::CreateLambda([this]() -> float
					{
						if (!DebugSubsystem.IsValid()) return 0.f;
						return DebugSubsystem->GetCMCParams().BrakingDeceleration;
					}),
					[this](float Val)
					{
						if (!DebugSubsystem.IsValid()) return;
						FLTCMCParams P = DebugSubsystem->GetCMCParams();
						P.BrakingDeceleration = Val;
						DebugSubsystem->SetCMCParams(P);
						if (CachedCharacter.IsValid())
						{
							CachedCharacter->GetCharacterMovement()->BrakingDecelerationWalking = Val;
						}
					})
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				BuildSliderRow(
					LOCTEXT("Speed", "移动速度"),
					PS ? PS->MaxWalkSpeedRange.Min : 0.f,
					PS ? PS->MaxWalkSpeedRange.Max : 1200.f,
					TAttribute<float>::CreateLambda([this]() -> float
					{
						if (!DebugSubsystem.IsValid()) return 0.f;
						return DebugSubsystem->GetCMCParams().MaxWalkSpeed;
					}),
					[this](float Val)
					{
						if (!DebugSubsystem.IsValid()) return;
						FLTCMCParams P = DebugSubsystem->GetCMCParams();
						P.MaxWalkSpeed = Val;
						DebugSubsystem->SetCMCParams(P);
						if (CachedCharacter.IsValid())
						{
							CachedCharacter->GetCharacterMovement()->MaxWalkSpeed = Val;
						}
					})
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				BuildSliderRow(
					LOCTEXT("CameraArm", "相机距离"),
					PS ? PS->CameraArmLengthRange.Min : 100.f,
					PS ? PS->CameraArmLengthRange.Max : 800.f,
					TAttribute<float>::CreateLambda([this]() -> float
					{
						if (!DebugSubsystem.IsValid()) return 0.f;
						return DebugSubsystem->GetCMCParams().CameraArmLength;
					}),
					[this](float Val)
					{
						if (!DebugSubsystem.IsValid()) return;
						FLTCMCParams P = DebugSubsystem->GetCMCParams();
						P.CameraArmLength = Val;
						DebugSubsystem->SetCMCParams(P);
						if (CachedCharacter.IsValid() && CachedCharacter->CameraBoom)
						{
							CachedCharacter->CameraBoom->TargetArmLength = Val;
						}
					})
			]

			// --- Toggles ---
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AnimHeader", "动画开关"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
			]

			// BlendSpace Loop (master lock)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("BSLoop", "BlendSpace 循环"),
					TEXT("bUseBlendSpaceLoop"))
			]

			// Start
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("Start", "起步动画"),
					TEXT("bEnableStart"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Start DM (conditional)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildConditionalToggleRow(
					LOCTEXT("StartDM", "  起步距离适配"),
					TEXT("bEnableStartDM"),
					StartDMRow,
					TAttribute<bool>::CreateLambda([this]() { return IsStartEnabled() && !IsBlendSpaceMode(); }))
			]

			// Start StrideWarping (conditional)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildConditionalToggleRow(
					LOCTEXT("StartSW", "  起步步幅修正"),
					TEXT("bEnableStartStrideWarping"),
					StartDMRow,
					TAttribute<bool>::CreateLambda([this]() { return IsStartEnabled() && !IsBlendSpaceMode(); }))
			]

			// Stop
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("Stop", "停步动画"),
					TEXT("bEnableStop"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Stop DM (conditional)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildConditionalToggleRow(
					LOCTEXT("StopDM", "  停步距离适配"),
					TEXT("bEnableStopDM"),
					StopDMRow,
					TAttribute<bool>::CreateLambda([this]() { return IsStopEnabled() && !IsBlendSpaceMode(); }))
			]

			// Stop StrideWarping (conditional)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildConditionalToggleRow(
					LOCTEXT("StopSW", "  停步步幅修正"),
					TEXT("bEnableStopStrideWarping"),
					StopDMRow,
					TAttribute<bool>::CreateLambda([this]() { return IsStopEnabled() && !IsBlendSpaceMode(); }))
			]

			// Cycle DM
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("CycleDM", "循环距离适配"),
					TEXT("bEnableCycleDM"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Cycle StrideWarping
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("CycleSW", "循环步幅修正"),
					TEXT("bEnableCycleStrideWarping"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Pivot
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("Pivot", "转向动画"),
					TEXT("bEnablePivot"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Pivot StrideWarping (conditional)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildConditionalToggleRow(
					LOCTEXT("PivotSW", "  转向步幅修正"),
					TEXT("bEnablePivotStrideWarping"),
					PivotSWRow,
					TAttribute<bool>::CreateLambda([this]() { return IsPivotEnabled() && !IsBlendSpaceMode(); }))
			]

			// AO
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("AO", "瞄准偏移"),
					TEXT("bEnableAimOffset"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// TIP (depends on AO)
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("TIP", "原地转向"),
					TEXT("bEnableTurnInPlace"),
					TAttribute<bool>::CreateLambda([this]() { return IsAOEnabled() && !IsBlendSpaceMode(); }))
			]

			// FootIK
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildToggleRow(
					LOCTEXT("FootIK", "脚步IK"),
					TEXT("bEnableFootIK"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Lean
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				BuildToggleRow(
					LOCTEXT("Lean", "身体倾斜"),
					TEXT("bEnableLean"),
					TAttribute<bool>::CreateLambda([this]() { return !IsBlendSpaceMode(); }))
			]

			// Character Sets
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				BuildCharacterSetSection()
			]

			// Reset
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 6)
			[
				SNew(SButton)
				.Text(LOCTEXT("Reset", "重置默认值"))
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]() -> FReply
				{
					if (DebugSubsystem.IsValid())
					{
						DebugSubsystem->ResetToDefaults();
					}
					return FReply::Handled();
				})
			]

			// Status
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildStatusSection()
			]
		]
	];
}

void SLTDebugPanel::SetCharacter(TWeakObjectPtr<ALTCharacter> InCharacter)
{
	CachedCharacter = InCharacter;
	if (InCharacter.IsValid())
	{
		DebugSubsystem = InCharacter->GetGameInstance()->GetSubsystem<ULTDebugSubsystem>();

		if (DebugSubsystem.IsValid())
		{
			FLTCMCParams P = DebugSubsystem->GetCMCParams();
			auto* CMC = InCharacter->GetCharacterMovement();
			P.MaxAcceleration = CMC->MaxAcceleration;
			P.BrakingDeceleration = CMC->BrakingDecelerationWalking;
			P.MaxWalkSpeed = CMC->MaxWalkSpeed;
			if (InCharacter->CameraBoom)
			{
				P.CameraArmLength = InCharacter->CameraBoom->TargetArmLength;
			}
			DebugSubsystem->SetCMCParams(P);
			DebugSubsystem->CaptureCurrentState(
				InCharacter->GetCurrentCharacterSetIndex(),
				InCharacter->bStrafeMovement,
				false
			);
		}
	}
}

// --- Slider ---

TSharedRef<SWidget> SLTDebugPanel::BuildSliderRow(
	const FText& Label,
	float MinValue, float MaxValue,
	TAttribute<float> ValueAttr,
	TFunction<void(float)> OnChanged)
{
	TSharedPtr<SSlider> Slider;
	TSharedPtr<STextBlock> ValueText;

	auto Row = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
		]
		+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center).Padding(4, 2)
		[
			SAssignNew(Slider, SSlider)
			.Value_Lambda([MinValue, MaxValue, ValueAttr]() -> float
			{
				if (FMath::IsNearlyEqual(MinValue, MaxValue)) return 0.f;
				const float ActualValue = ValueAttr.Get();
				return FMath::Clamp((ActualValue - MinValue) / (MaxValue - MinValue), 0.f, 1.f);
			})
			.OnValueChanged_Lambda([MinValue, MaxValue, OnChanged](float Normalized)
			{
				float Denorm = MinValue + Normalized * (MaxValue - MinValue);
				OnChanged(Denorm);
			})
		]
		+ SHorizontalBox::Slot().FillWidth(0.15f).VAlign(VAlign_Center).Padding(4, 0, 0, 0)
		[
			SAssignNew(ValueText, STextBlock)
			.Text_Lambda([ValueAttr]() -> FText
			{
				return FText::FromString(FString::Printf(TEXT("%.0f"), ValueAttr.Get()));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f))
		];

	return Row;
}

// --- Toggle ---

TSharedRef<SWidget> SLTDebugPanel::BuildToggleRow(
	const FText& Label,
	FName PropertyName,
	TAttribute<bool> IsEnabledAttr)
{
	return SNew(SHorizontalBox)
		.IsEnabled(IsEnabledAttr)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this, PropertyName]() -> ECheckBoxState
			{
				return IsToggleChecked(PropertyName);
			})
			.OnCheckStateChanged_Lambda([this, PropertyName](ECheckBoxState NewState)
			{
				OnToggleCheckChanged(PropertyName, NewState);
			})
		]
		+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(6, 2)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor::White)
		];
}

TSharedRef<SWidget> SLTDebugPanel::BuildConditionalToggleRow(
	const FText& Label,
	FName PropertyName,
	TSharedPtr<SVerticalBox>& RowSlot,
	TAttribute<bool> IsEnabledAttr)
{
	TSharedPtr<SWidget> RowWidget;
	RowSlot = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(RowWidget, SBorder)
			.BorderBackgroundColor(FLinearColor::Transparent)
			[
				BuildToggleRow(Label, PropertyName, IsEnabledAttr)
			]
		];

	RowSlot->SetVisibility(TAttribute<EVisibility>::CreateLambda([this, IsEnabledAttr]() -> EVisibility
	{
		if (!IsEnabledAttr.Get()) return EVisibility::Collapsed;
		return EVisibility::Visible;
	}));

	return RowSlot.ToSharedRef();
}

// --- State Queries ---

ECheckBoxState SLTDebugPanel::IsToggleChecked(FName PropertyName) const
{
	const ULASettings* Settings = ULASettings::GetConst();
	if (!Settings) return ECheckBoxState::Unchecked;

	const FLAAnimToggleSettings& S = Settings->GetToggleSettings();
	bool bVal = false;

	if (PropertyName == TEXT("bUseBlendSpaceLoop")) bVal = S.bUseBlendSpaceLoop;
	else if (PropertyName == TEXT("bEnableStart")) bVal = S.bEnableStart;
	else if (PropertyName == TEXT("bEnableStartDM")) bVal = S.bEnableStartDM;
	else if (PropertyName == TEXT("bEnableStartStrideWarping")) bVal = S.bEnableStartStrideWarping;
	else if (PropertyName == TEXT("bEnableStop")) bVal = S.bEnableStop;
	else if (PropertyName == TEXT("bEnableStopDM")) bVal = S.bEnableStopDM;
	else if (PropertyName == TEXT("bEnableStopStrideWarping")) bVal = S.bEnableStopStrideWarping;
	else if (PropertyName == TEXT("bEnableCycleDM")) bVal = S.bEnableCycleDM;
	else if (PropertyName == TEXT("bEnableCycleStrideWarping")) bVal = S.bEnableCycleStrideWarping;
	else if (PropertyName == TEXT("bEnablePivot")) bVal = S.bEnablePivot;
	else if (PropertyName == TEXT("bEnablePivotStrideWarping")) bVal = S.bEnablePivotStrideWarping;
	else if (PropertyName == TEXT("bEnableAimOffset")) bVal = S.bEnableAimOffset;
	else if (PropertyName == TEXT("bEnableTurnInPlace")) bVal = S.bEnableTurnInPlace;
	else if (PropertyName == TEXT("bEnableFootIK")) bVal = S.bEnableFootIK;
	else if (PropertyName == TEXT("bEnableLean")) bVal = S.bEnableLean;

	return bVal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SLTDebugPanel::OnToggleCheckChanged(FName PropertyName, ECheckBoxState NewState)
{
	ULASettings* Settings = ULASettings::Get();
	if (!Settings) return;

	FLAAnimToggleSettings S = Settings->GetToggleSettings();
	const bool bNew = (NewState == ECheckBoxState::Checked);

	if (PropertyName == TEXT("bUseBlendSpaceLoop")) S.bUseBlendSpaceLoop = bNew;
	else if (PropertyName == TEXT("bEnableStart")) S.bEnableStart = bNew;
	else if (PropertyName == TEXT("bEnableStartDM")) S.bEnableStartDM = bNew;
	else if (PropertyName == TEXT("bEnableStartStrideWarping")) S.bEnableStartStrideWarping = bNew;
	else if (PropertyName == TEXT("bEnableStop")) S.bEnableStop = bNew;
	else if (PropertyName == TEXT("bEnableStopDM")) S.bEnableStopDM = bNew;
	else if (PropertyName == TEXT("bEnableStopStrideWarping")) S.bEnableStopStrideWarping = bNew;
	else if (PropertyName == TEXT("bEnableCycleDM")) S.bEnableCycleDM = bNew;
	else if (PropertyName == TEXT("bEnableCycleStrideWarping")) S.bEnableCycleStrideWarping = bNew;
	else if (PropertyName == TEXT("bEnablePivot")) S.bEnablePivot = bNew;
	else if (PropertyName == TEXT("bEnablePivotStrideWarping")) S.bEnablePivotStrideWarping = bNew;
	else if (PropertyName == TEXT("bEnableAimOffset")) S.bEnableAimOffset = bNew;
	else if (PropertyName == TEXT("bEnableTurnInPlace")) S.bEnableTurnInPlace = bNew;
	else if (PropertyName == TEXT("bEnableFootIK")) S.bEnableFootIK = bNew;
	else if (PropertyName == TEXT("bEnableLean")) S.bEnableLean = bNew;

	Settings->SetToggleSettings(S);

	if (DebugSubsystem.IsValid())
	{
		FLTDebugState DS = DebugSubsystem->GetDebugState();
		DS.ToggleSettings = S;
		DebugSubsystem->SetDebugState(DS);
	}
}

bool SLTDebugPanel::IsBlendSpaceMode() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings().bUseBlendSpaceLoop : true;
}

bool SLTDebugPanel::IsStartEnabled() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings().bEnableStart : false;
}

bool SLTDebugPanel::IsStopEnabled() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings().bEnableStop : false;
}

bool SLTDebugPanel::IsPivotEnabled() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings().bEnablePivot : false;
}

bool SLTDebugPanel::IsAOEnabled() const
{
	const ULASettings* Settings = ULASettings::GetConst();
	return Settings ? Settings->GetToggleSettings().bEnableAimOffset : false;
}

void SLTDebugPanel::UpdateConditionalVisibility()
{
	const ULASettings* Settings = ULASettings::GetConst();
	if (!Settings) return;
	const FLAAnimToggleSettings& S = Settings->GetToggleSettings();

	if (StartDMRow.IsValid())
	{
		StartDMRow->SetVisibility(S.bEnableStart ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (StopDMRow.IsValid())
	{
		StopDMRow->SetVisibility(S.bEnableStop ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (PivotSWRow.IsValid())
	{
		PivotSWRow->SetVisibility(S.bEnablePivot ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

TSharedRef<SWidget> SLTDebugPanel::BuildCharacterSetSection()
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	CharacterSetIconBrushes.Reset();

	Box->AddSlot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CharacterSetHeader", "角色列表"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
	];

	if (!Settings || Settings->CharacterSets.Num() == 0)
	{
		Box->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoCharacterSets", "未配置角色列表"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FLinearColor::Gray)
		];
		return Box;
	}

	for (int32 Index = 0; Index < Settings->CharacterSets.Num(); ++Index)
	{
		CharacterSetIconBrushes.Add(nullptr);
		Box->AddSlot().AutoHeight().Padding(0, 1)
		[
			SNew(SButton)
			.ButtonColorAndOpacity(this, &SLTDebugPanel::GetCharacterSetButtonColor, Index)
			.OnClicked(this, &SLTDebugPanel::OnCharacterSetClicked, Index)
			[
				BuildCharacterSetButtonContent(Index)
			]
		];
	}

	return Box;
}

TSharedRef<SWidget> SLTDebugPanel::BuildCharacterSetButtonContent(int32 Index)
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	const ULTCharacterSet* CharacterSet = nullptr;
	if (Settings && Settings->CharacterSets.IsValidIndex(Index))
	{
		CharacterSet = Settings->CharacterSets[Index].LoadSynchronous();
	}

	if (CharacterSet && CharacterSet->Icon)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(CharacterSet->Icon);
		Brush->ImageSize = FVector2D(24.0f, 24.0f);
		if (CharacterSetIconBrushes.IsValidIndex(Index))
		{
			CharacterSetIconBrushes[Index] = Brush;
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)
			[
				SNew(SImage)
				.Image(Brush.Get())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SLTDebugPanel::GetCharacterSetDisplayName, Index)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			];
	}

	return SNew(STextBlock)
		.Text(this, &SLTDebugPanel::GetCharacterSetDisplayName, Index)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9));
}

FReply SLTDebugPanel::OnCharacterSetClicked(int32 Index)
{
	if (CachedCharacter.IsValid())
	{
		CachedCharacter->ApplyCharacterSet(Index);
	}

	return FReply::Handled();
}

FSlateColor SLTDebugPanel::GetCharacterSetButtonColor(int32 Index) const
{
	if (CachedCharacter.IsValid() && CachedCharacter->GetCurrentCharacterSetIndex() == Index)
	{
		return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f, 1.0f));
	}

	return FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}

FText SLTDebugPanel::GetCharacterSetDisplayName(int32 Index) const
{
	const ULTProjectSettings* Settings = ULTProjectSettings::Get();
	if (!Settings || !Settings->CharacterSets.IsValidIndex(Index))
	{
		return FText::FromString(FString::Printf(TEXT("Character %d"), Index + 1));
	}

	const ULTCharacterSet* CharacterSet = Settings->CharacterSets[Index].LoadSynchronous();
	if (CharacterSet && !CharacterSet->DisplayName.IsEmpty())
	{
		return CharacterSet->DisplayName;
	}

	return FText::FromString(FString::Printf(TEXT("Character %d"), Index + 1));
}

// --- Status ---

TSharedRef<SWidget> SLTDebugPanel::BuildStatusSection()
{
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
		.Padding(6)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("SpeedL", "速度: "))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor::Gray)
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetSpeedText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity_Lambda([this]() { return GetStatusColor(); })
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("StateL", "状态: "))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor::Gray)
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetMovementStateText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("StrafeL", "瞄准行走: "))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor::Gray)
				]
				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetStrafeText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity_Lambda([this]() { return GetStatusColor(); })
				]
			]
		];
}

FText SLTDebugPanel::GetSpeedText() const
{
	if (!CachedCharacter.IsValid()) return FText::GetEmpty();
	return FText::FromString(FString::Printf(TEXT("%.0f"), CachedCharacter->GetVelocity().Size2D()));
}

FText SLTDebugPanel::GetMovementStateText() const
{
	if (!CachedCharacter.IsValid()) return FText::GetEmpty();
	if (CachedCharacter->GetCharacterMovement()->IsFalling()) return LOCTEXT("InAir", "空中");
	if (CachedCharacter->GetVelocity().Size2D() > KINDA_SMALL_NUMBER) return LOCTEXT("Moving", "移动中");
	return LOCTEXT("Idle", "静止");
}

FText SLTDebugPanel::GetStrafeText() const
{
	if (!CachedCharacter.IsValid()) return FText::GetEmpty();
	return CachedCharacter->bStrafeMovement ? LOCTEXT("On", "开启") : LOCTEXT("Off", "关闭");
}

FSlateColor SLTDebugPanel::GetStatusColor() const
{
	if (!CachedCharacter.IsValid()) return FSlateColor(FLinearColor::White);
	return CachedCharacter->GetVelocity().Size2D() > KINDA_SMALL_NUMBER
		? FSlateColor(FLinearColor(0.4f, 1.0f, 0.4f))
		: FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f));
}

#undef LOCTEXT_NAMESPACE
