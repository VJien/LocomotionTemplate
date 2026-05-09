#pragma once

#include "CoreMinimal.h"
#include "Debug/LAAnimDebuggerTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"

class FLAAnimDebuggerViewModel;
class SScrollBox;
template <typename ItemType> class STreeView;
template <typename ItemType> class SListView;

class SLAAnimDebuggerWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLAAnimDebuggerWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SLAAnimDebuggerWindow() override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildActorTree();
	TSharedRef<SWidget> BuildPropertyPanel();
	TSharedRef<SWidget> BuildLogPanel();

	TSharedRef<ITableRow> GenerateActorTreeRow(TSharedPtr<FLAAnimDebuggerTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetActorTreeChildren(TSharedPtr<FLAAnimDebuggerTreeItem> Item, TArray<TSharedPtr<FLAAnimDebuggerTreeItem>>& OutChildren) const;
	void OnActorTreeSelectionChanged(TSharedPtr<FLAAnimDebuggerTreeItem> Item, ESelectInfo::Type SelectInfo);

	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FLAAnimDebuggerLogItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void RebuildPropertyPanel();
	TSharedRef<SWidget> BuildPropertyRow(const FLAAnimDebuggerPropertyItem& Item);
	FText GetEmptyStateText() const;

private:
	TSharedPtr<FLAAnimDebuggerViewModel> ViewModel;
	TSharedPtr<STreeView<TSharedPtr<FLAAnimDebuggerTreeItem>>> ActorTreeView;
	TSharedPtr<SScrollBox> PropertyScrollBox;
	TSharedPtr<SListView<TSharedPtr<FLAAnimDebuggerLogItem>>> LogListView;
	FDelegateHandle PostPIEStartedHandle;
	int32 LastRenderedPropertiesRevision = INDEX_NONE;
};
