#include "Character/LTHUD.h"
#include "Character/LTCharacter.h"
#include "Core/LTProjectSettings.h"
#include "UI/SLTDebugPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/GameInstance.h"

ALTHUD::ALTHUD()
{
	DebugHintText = FText::FromString(TEXT("Press [Tab] to toggle Debug Panel"));

	ControlHints = FText::FromString(TEXT(
		"[WASD] Move  |  [Mouse] Look  |  [Space] Jump  |  "
		"[B] Strafe  |  [V] Front Camera  |  [Tab] Debug Panel"
	));
}

void ALTHUD::BeginPlay()
{
	Super::BeginPlay();

	if (bShowDebugHint)
	{
		ShowHintBar();
	}
}

void ALTHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideDebugPanel();
	HideHintBar();
	Super::EndPlay(EndPlayReason);
}

void ALTHUD::ToggleDebugPanel()
{
	if (bDebugPanelVisible)
	{
		HideDebugPanel();
	}
	else
	{
		ShowDebugPanel();
	}
}

void ALTHUD::ShowDebugPanel()
{
	if (DebugPanel.IsValid()) return;

	DebugPanel = SNew(SLTDebugPanel);

	if (ALTCharacter* Char = Cast<ALTCharacter>(GetOwningPawn()))
	{
		DebugPanel->SetCharacter(Char);
	}

	GEngine->GameViewport->AddViewportWidgetContent(
		DebugPanel.ToSharedRef(),
		100
	);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		PC->bShowMouseCursor = true;
	}

	bDebugPanelVisible = true;
}

void ALTHUD::HideDebugPanel()
{
	if (!DebugPanel.IsValid()) return;

	GEngine->GameViewport->RemoveViewportWidgetContent(
		DebugPanel.ToSharedRef()
	);

	DebugPanel.Reset();
	bDebugPanelVisible = false;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
		PC->bShowMouseCursor = false;
	}
}

void ALTHUD::ShowHintBar()
{
	if (HintWidget.IsValid()) return;

	HintWidget = SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0, 6, 0, 0))
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.5f))
			.Padding(FMargin(12, 4, 12, 4))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(ControlHints)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 2, 0, 0)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() -> FText
					{
						const ALTCharacter* Character = Cast<ALTCharacter>(GetOwningPawn());
						return Character && Character->bStrafeMovement
							? FText::FromString(TEXT("Mode: Strafe"))
							: FText::FromString(TEXT("Mode: Orient To Movement"));
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity_Lambda([this]() -> FSlateColor
					{
						const ALTCharacter* Character = Cast<ALTCharacter>(GetOwningPawn());
						return Character && Character->bStrafeMovement
							? FSlateColor(FLinearColor(0.25f, 0.9f, 1.0f, 1.0f))
							: FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.0f));
					})
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 2, 0, 0)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() -> FText
					{
						const ALTCharacter* Character = Cast<ALTCharacter>(GetOwningPawn());
						if (!Character) return FText::GetEmpty();
						const float Speed = Character->GetCharacterMovement()->MaxWalkSpeed;
						return FText::FromString(FString::Printf(TEXT("Speed: %.0f"), Speed));
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0, 4, 8, 0))
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.3f))
			.Padding(FMargin(8, 3, 8, 3))
			.Visibility_Lambda([this]() -> EVisibility
			{
				return bDebugPanelVisible ? EVisibility::Collapsed : EVisibility::Visible;
			})
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("[Tab] Debug")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
			]
		];

	GEngine->GameViewport->AddViewportWidgetContent(
		HintWidget.ToSharedRef(),
		0
	);
}

void ALTHUD::HideHintBar()
{
	if (!HintWidget.IsValid()) return;

	GEngine->GameViewport->RemoveViewportWidgetContent(
		HintWidget.ToSharedRef()
	);

	HintWidget.Reset();
}
