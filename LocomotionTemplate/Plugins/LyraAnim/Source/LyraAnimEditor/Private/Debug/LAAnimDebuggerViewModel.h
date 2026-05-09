#pragma once

#include "CoreMinimal.h"
#include "Debug/LAAnimDebuggerTypes.h"

class FLAAnimDebuggerViewModel : public TSharedFromThis<FLAAnimDebuggerViewModel>
{
public:
	FLAAnimDebuggerViewModel();
	~FLAAnimDebuggerViewModel();

	void RefreshActorTree();
	const TArray<TSharedPtr<FLAAnimDebuggerTreeItem>>& GetRootItems() const { return RootItems; }

	void SetSelectedItem(TSharedPtr<FLAAnimDebuggerTreeItem> InItem);
	TSharedPtr<FLAAnimDebuggerTreeItem> GetSelectedItem() const { return SelectedItem; }

	void RebuildProperties();
	const TArray<FLAAnimDebuggerPropertyItem>& GetProperties() const { return Properties; }
	int32 GetPropertiesRevision() const { return PropertiesRevision; }
	FString GetPropertyValueText(const FString& Key) const;

	void SetSearchText(const FString& InSearchText);
	void SetRefreshInterval(float InRefreshInterval);
	float GetRefreshInterval() const { return RefreshInterval; }

	void SetFollowEditorSelection(bool bInFollowEditorSelection);
	bool IsFollowingEditorSelection() const { return bFollowEditorSelection; }
	bool TryFollowEditorSelection();

	void SetRecorded(const FString& Key, bool bRecorded);
	bool IsRecorded(const FString& Key) const;

	void ClearLog();
	const TArray<TSharedPtr<FLAAnimDebuggerLogItem>>& GetLogs() const { return Logs; }

	void Tick(double CurrentTimeSeconds);

private:
	UWorld* FindDebugWorld() const;
	void AddActorIfValid(AActor* Actor);
	void AddMeshChildren(const TSharedPtr<FLAAnimDebuggerTreeItem>& ActorItem, AActor* Actor);
	void AddAnimInstanceChildren(const TSharedPtr<FLAAnimDebuggerTreeItem>& MeshItem, USkeletalMeshComponent* Mesh);
	void GatherTargetInstances(TArray<UAnimInstance*>& OutInstances) const;
	void GatherPropertiesForInstance(UAnimInstance* Instance, TArray<FLAAnimDebuggerPropertyItem>& OutProperties) const;
	FString GetPropertyValueText(FProperty* Property, UObject* Container) const;
	FLinearColor GetPropertyTypeColor(FProperty* Property) const;
	bool ShouldDisplayProperty(FProperty* Property) const;
	void UpdateLogs(double CurrentTimeSeconds, const TArray<FLAAnimDebuggerPropertyItem>& NewProperties);
	void LoadSettings();
	void SaveSettings() const;

private:
	TArray<TSharedPtr<FLAAnimDebuggerTreeItem>> RootItems;
	TSharedPtr<FLAAnimDebuggerTreeItem> SelectedItem;
	TArray<FLAAnimDebuggerPropertyItem> Properties;
	TArray<TSharedPtr<FLAAnimDebuggerLogItem>> Logs;
	TMap<FString, FString> LastValues;
	TSet<FString> RecordedKeys;
	FString SearchText;
	float RefreshInterval = 0.1f;
	bool bFollowEditorSelection = true;
	double LastRefreshTimeSeconds = 0.0;
	int32 PropertiesRevision = 0;
	FString LastPropertiesLayoutSignature;
	static constexpr int32 MaxLogCount = 1000;
};
