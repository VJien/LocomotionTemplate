#include "Debug/SLAAnimDebuggerWindow.h"

#include "Debug/LAAnimDebuggerViewModel.h"
#include "Editor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"

#define LOCTEXT_NAMESPACE "SLAAnimDebuggerWindow"

void SLAAnimDebuggerWindow::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FLAAnimDebuggerViewModel>();
	ViewModel->RefreshActorTree();

	PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddLambda([this](bool bIsSimulating)
	{
		if (ViewModel.IsValid())
		{
			ViewModel->RefreshActorTree();
			LastRenderedPropertiesRevision = INDEX_NONE;
			if (ActorTreeView.IsValid())
			{
				ActorTreeView->RequestTreeRefresh();
			}
			RebuildPropertyPanel();
		}
	});

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildToolbar()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			+ SSplitter::Slot()
			.Value(0.22f)
			[
				BuildActorTree()
			]
			+ SSplitter::Slot()
			.Value(0.53f)
			[
				BuildPropertyPanel()
			]
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				BuildLogPanel()
			]
		]
	];

	RebuildPropertyPanel();
}

SLAAnimDebuggerWindow::~SLAAnimDebuggerWindow()
{
	if (PostPIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);
	}
}

void SLAAnimDebuggerWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!ViewModel.IsValid())
	{
		return;
	}

	ViewModel->Tick(InCurrentTime);
	RebuildPropertyPanel();

	if (LogListView.IsValid())
	{
		LogListView->RequestListRefresh();
	}
}

TSharedRef<SWidget> SLAAnimDebuggerWindow::BuildToolbar()
{
	return SNew(SBorder)
		.Padding(4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 6, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh Actors"))
				.OnClicked_Lambda([this]()
				{
					ViewModel->RefreshActorTree();
					LastRenderedPropertiesRevision = INDEX_NONE;
					if (ActorTreeView.IsValid())
					{
						ActorTreeView->RequestTreeRefresh();
					}
					RebuildPropertyPanel();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 6, 0)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]()
				{
					return ViewModel->IsFollowingEditorSelection() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
				{
					ViewModel->SetFollowEditorSelection(State == ECheckBoxState::Checked);
				})
				[
					SNew(STextBlock).Text(LOCTEXT("FollowSelection", "Follow Editor Selection"))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0, 0, 6, 0)
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("Search", "Search property or category"))
				.OnTextChanged_Lambda([this](const FText& Text)
				{
					ViewModel->SetSearchText(Text.ToString());
					LastRenderedPropertiesRevision = INDEX_NONE;
					RebuildPropertyPanel();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Rate", "Rate"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 6, 0)
			[
				SNew(SSpinBox<float>)
				.MinValue(0.05f)
				.MaxValue(2.0f)
				.Value_Lambda([this]() { return ViewModel->GetRefreshInterval(); })
				.OnValueCommitted_Lambda([this](float Value, ETextCommit::Type CommitType)
				{
					ViewModel->SetRefreshInterval(Value);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearLog", "Clear Log"))
				.OnClicked_Lambda([this]()
				{
					ViewModel->ClearLog();
					if (LogListView.IsValid())
					{
						LogListView->RequestListRefresh();
					}
					return FReply::Handled();
				})
			]
		];
}

TSharedRef<SWidget> SLAAnimDebuggerWindow::BuildActorTree()
{
	return SNew(SBorder)
		.Padding(4.0f)
		[
			SAssignNew(ActorTreeView, STreeView<TSharedPtr<FLAAnimDebuggerTreeItem>>)
			.TreeItemsSource(&ViewModel->GetRootItems())
			.OnGenerateRow(this, &SLAAnimDebuggerWindow::GenerateActorTreeRow)
			.OnGetChildren(this, &SLAAnimDebuggerWindow::GetActorTreeChildren)
			.OnSelectionChanged(this, &SLAAnimDebuggerWindow::OnActorTreeSelectionChanged)
		];
}

TSharedRef<ITableRow> SLAAnimDebuggerWindow::GenerateActorTreeRow(TSharedPtr<FLAAnimDebuggerTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FLAAnimDebuggerTreeItem>>, OwnerTable)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Item.IsValid() ? Item->DisplayName : TEXT("Invalid")))
	];
}

void SLAAnimDebuggerWindow::GetActorTreeChildren(TSharedPtr<FLAAnimDebuggerTreeItem> Item, TArray<TSharedPtr<FLAAnimDebuggerTreeItem>>& OutChildren) const
{
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

void SLAAnimDebuggerWindow::OnActorTreeSelectionChanged(TSharedPtr<FLAAnimDebuggerTreeItem> Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid())
	{
		ViewModel->SetSelectedItem(Item);
		LastRenderedPropertiesRevision = INDEX_NONE;
		RebuildPropertyPanel();
	}
}

TSharedRef<SWidget> SLAAnimDebuggerWindow::BuildPropertyPanel()
{
	return SNew(SBorder)
		.Padding(4.0f)
		[
			SAssignNew(PropertyScrollBox, SScrollBox)
		];
}

TSharedRef<SWidget> SLAAnimDebuggerWindow::BuildLogPanel()
{
	return SNew(SBorder)
		.Padding(4.0f)
		[
			SAssignNew(LogListView, SListView<TSharedPtr<FLAAnimDebuggerLogItem>>)
			.ListItemsSource(&ViewModel->GetLogs())
			.OnGenerateRow(this, &SLAAnimDebuggerWindow::GenerateLogRow)
		];
}

TSharedRef<ITableRow> SLAAnimDebuggerWindow::GenerateLogRow(TSharedPtr<FLAAnimDebuggerLogItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString Text = Item.IsValid()
		? FString::Printf(TEXT("%.3f  %s / %s.%s  %s -> %s"), Item->TimeSeconds, *Item->InstanceName, *Item->Category, *Item->PropertyName, *Item->OldValue, *Item->NewValue)
		: TEXT("Invalid log");

	return SNew(STableRow<TSharedPtr<FLAAnimDebuggerLogItem>>, OwnerTable)
	[
		SNew(STextBlock).Text(FText::FromString(Text))
	];
}

void SLAAnimDebuggerWindow::RebuildPropertyPanel()
{
	if (!PropertyScrollBox.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	if (LastRenderedPropertiesRevision == ViewModel->GetPropertiesRevision())
	{
		return;
	}
	LastRenderedPropertiesRevision = ViewModel->GetPropertiesRevision();

	PropertyScrollBox->ClearChildren();

	const TArray<FLAAnimDebuggerPropertyItem>& Items = ViewModel->GetProperties();
	if (Items.Num() == 0)
	{
		PropertyScrollBox->AddSlot()
		[
			SNew(STextBlock).Text(GetEmptyStateText())
		];
		return;
	}

	TMap<FString, TArray<FLAAnimDebuggerPropertyItem>> PropertiesByInstanceAndCategory;
	for (const FLAAnimDebuggerPropertyItem& Item : Items)
	{
		const FString GroupKey = FString::Printf(TEXT("%s|%s"), *Item.InstanceName, *Item.Category);
		PropertiesByInstanceAndCategory.FindOrAdd(GroupKey).Add(Item);
	}

	for (const TPair<FString, TArray<FLAAnimDebuggerPropertyItem>>& Pair : PropertiesByInstanceAndCategory)
	{
		FString InstanceName;
		FString Category;
		Pair.Key.Split(TEXT("|"), &InstanceName, &Category);

		TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
		for (const FLAAnimDebuggerPropertyItem& Item : Pair.Value)
		{
			Rows->AddSlot().AutoHeight()[BuildPropertyRow(Item)];
		}

		PropertyScrollBox->AddSlot()
		.Padding(0, 0, 0, 4)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(false)
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s / %s"), *InstanceName, *Category)))
			]
			.BodyContent()
			[
				Rows
			]
		];
	}
}

TSharedRef<SWidget> SLAAnimDebuggerWindow::BuildPropertyRow(const FLAAnimDebuggerPropertyItem& Item)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, 6, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("o")))
			.ColorAndOpacity(Item.TypeColor)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.45f)
		[
			SNew(STextBlock).Text(FText::FromString(Item.PropertyName))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.45f)
		[
			SNew(STextBlock)
			.Text_Lambda([ViewModel = ViewModel, Key = Item.Key]()
			{
				return ViewModel.IsValid()
					? FText::FromString(ViewModel->GetPropertyValueText(Key))
					: FText::GetEmpty();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([ViewModel = ViewModel, Key = Item.Key]()
			{
				return ViewModel.IsValid() && ViewModel->IsRecorded(Key)
					? ECheckBoxState::Checked
					: ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([ViewModel = ViewModel, Key = Item.Key](ECheckBoxState State)
			{
				if (ViewModel.IsValid())
				{
					ViewModel->SetRecorded(Key, State == ECheckBoxState::Checked);
				}
			})
			[
				SNew(STextBlock).Text(LOCTEXT("Record", "Record"))
			]
		];
}

FText SLAAnimDebuggerWindow::GetEmptyStateText() const
{
	return LOCTEXT("EmptyState", "No AnimBlueprint properties. Start PIE, select an Actor with a SkeletalMeshComponent, or click Refresh Actors.");
}

#undef LOCTEXT_NAMESPACE
