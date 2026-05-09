#include "Debug/LAAnimDebuggerViewModel.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/ConfigCacheIni.h"

namespace LAAnimDebugger
{
static const TCHAR* SettingsSection = TEXT("LyraAnimDebugger");
}

FLAAnimDebuggerViewModel::FLAAnimDebuggerViewModel()
{
	LoadSettings();
}

FLAAnimDebuggerViewModel::~FLAAnimDebuggerViewModel()
{
	SaveSettings();
}

void FLAAnimDebuggerViewModel::RefreshActorTree()
{
	RootItems.Reset();

	UWorld* World = FindDebugWorld();
	if (!World)
	{
		SelectedItem.Reset();
		Properties.Reset();
		++PropertiesRevision;
		return;
	}

	for (FActorIterator It(World); It; ++It)
	{
		AddActorIfValid(*It);
	}

	if (!SelectedItem.IsValid() && RootItems.Num() > 0)
	{
		SelectedItem = RootItems[0];
	}

	RebuildProperties();
}

void FLAAnimDebuggerViewModel::SetSelectedItem(TSharedPtr<FLAAnimDebuggerTreeItem> InItem)
{
	SelectedItem = InItem;
	RebuildProperties();
}

void FLAAnimDebuggerViewModel::RebuildProperties()
{
	TArray<UAnimInstance*> Instances;
	GatherTargetInstances(Instances);

	TArray<FLAAnimDebuggerPropertyItem> NewProperties;
	for (UAnimInstance* Instance : Instances)
	{
		GatherPropertiesForInstance(Instance, NewProperties);
	}

	UpdateLogs(FPlatformTime::Seconds(), NewProperties);

	FString NewLayoutSignature;
	NewLayoutSignature.Reserve(NewProperties.Num() * 64);
	for (const FLAAnimDebuggerPropertyItem& Property : NewProperties)
	{
		NewLayoutSignature += Property.Key;
		NewLayoutSignature += TEXT("|");
		NewLayoutSignature += Property.InstanceName;
		NewLayoutSignature += TEXT("|");
		NewLayoutSignature += Property.Category;
		NewLayoutSignature += TEXT("|");
		NewLayoutSignature += Property.PropertyName;
		NewLayoutSignature += TEXT("\n");
	}

	Properties = MoveTemp(NewProperties);
	if (NewLayoutSignature != LastPropertiesLayoutSignature)
	{
		LastPropertiesLayoutSignature = MoveTemp(NewLayoutSignature);
		++PropertiesRevision;
	}
}

FString FLAAnimDebuggerViewModel::GetPropertyValueText(const FString& Key) const
{
	for (const FLAAnimDebuggerPropertyItem& Property : Properties)
	{
		if (Property.Key == Key)
		{
			return Property.ValueText;
		}
	}

	return TEXT("<missing>");
}

void FLAAnimDebuggerViewModel::SetSearchText(const FString& InSearchText)
{
	SearchText = InSearchText;
	RebuildProperties();
}

void FLAAnimDebuggerViewModel::SetRefreshInterval(float InRefreshInterval)
{
	RefreshInterval = FMath::Clamp(InRefreshInterval, 0.05f, 2.0f);
	SaveSettings();
}

void FLAAnimDebuggerViewModel::SetFollowEditorSelection(bool bInFollowEditorSelection)
{
	bFollowEditorSelection = bInFollowEditorSelection;
	SaveSettings();
}

bool FLAAnimDebuggerViewModel::TryFollowEditorSelection()
{
	if (!bFollowEditorSelection || !GEditor)
	{
		return false;
	}

	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection)
	{
		return false;
	}

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		AActor* SelectedActor = Cast<AActor>(*It);
		if (!SelectedActor)
		{
			continue;
		}

		for (const TSharedPtr<FLAAnimDebuggerTreeItem>& ActorItem : RootItems)
		{
			if (ActorItem.IsValid() && ActorItem->Actor.Get() == SelectedActor)
			{
				if (SelectedItem != ActorItem)
				{
					SelectedItem = ActorItem;
					RebuildProperties();
				}
				return true;
			}
		}
	}

	return false;
}

void FLAAnimDebuggerViewModel::SetRecorded(const FString& Key, bool bRecorded)
{
	if (bRecorded)
	{
		RecordedKeys.Add(Key);
	}
	else
	{
		RecordedKeys.Remove(Key);
	}
	SaveSettings();
	RebuildProperties();
}

bool FLAAnimDebuggerViewModel::IsRecorded(const FString& Key) const
{
	return RecordedKeys.Contains(Key);
}

void FLAAnimDebuggerViewModel::ClearLog()
{
	Logs.Reset();
}

void FLAAnimDebuggerViewModel::Tick(double CurrentTimeSeconds)
{
	if (CurrentTimeSeconds - LastRefreshTimeSeconds < RefreshInterval)
	{
		return;
	}

	LastRefreshTimeSeconds = CurrentTimeSeconds;
	TryFollowEditorSelection();
	RebuildProperties();
}

UWorld* FLAAnimDebuggerViewModel::FindDebugWorld() const
{
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				return Context.World();
			}
		}
	}

	return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}

void FLAAnimDebuggerViewModel::AddActorIfValid(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	TSharedPtr<FLAAnimDebuggerTreeItem> ActorItem = MakeShared<FLAAnimDebuggerTreeItem>();
	ActorItem->Type = ELAAnimDebuggerTreeItemType::Actor;
	ActorItem->Actor = Actor;
	ActorItem->DisplayName = Actor->GetActorLabel().IsEmpty() ? Actor->GetName() : Actor->GetActorLabel();

	AddMeshChildren(ActorItem, Actor);

	if (ActorItem->Children.Num() > 0)
	{
		RootItems.Add(ActorItem);
	}
}

void FLAAnimDebuggerViewModel::AddMeshChildren(const TSharedPtr<FLAAnimDebuggerTreeItem>& ActorItem, AActor* Actor)
{
	TArray<USkeletalMeshComponent*> MeshComponents;
	Actor->GetComponents(MeshComponents);

	for (USkeletalMeshComponent* Mesh : MeshComponents)
	{
		if (!Mesh || !Mesh->GetAnimInstance())
		{
			continue;
		}

		TSharedPtr<FLAAnimDebuggerTreeItem> MeshItem = MakeShared<FLAAnimDebuggerTreeItem>();
		MeshItem->Type = ELAAnimDebuggerTreeItemType::Mesh;
		MeshItem->Actor = Actor;
		MeshItem->Mesh = Mesh;
		MeshItem->DisplayName = Mesh->GetName();

		AddAnimInstanceChildren(MeshItem, Mesh);

		if (MeshItem->Children.Num() > 0)
		{
			ActorItem->Children.Add(MeshItem);
		}
	}
}

void FLAAnimDebuggerViewModel::AddAnimInstanceChildren(const TSharedPtr<FLAAnimDebuggerTreeItem>& MeshItem, USkeletalMeshComponent* Mesh)
{
	UAnimInstance* MainInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!MainInstance)
	{
		return;
	}

	TArray<UAnimInstance*> Instances;
	Instances.Add(MainInstance);
	const USkeletalMeshComponent* ConstMesh = Mesh;
	Instances.Append(ConstMesh->GetLinkedAnimInstances());

	TSet<UAnimInstance*> Seen;
	for (UAnimInstance* Instance : Instances)
	{
		if (!Instance || Seen.Contains(Instance))
		{
			continue;
		}

		Seen.Add(Instance);

		TSharedPtr<FLAAnimDebuggerTreeItem> InstanceItem = MakeShared<FLAAnimDebuggerTreeItem>();
		InstanceItem->Type = ELAAnimDebuggerTreeItemType::AnimInstance;
		InstanceItem->Actor = MeshItem->Actor;
		InstanceItem->Mesh = Mesh;
		InstanceItem->AnimInstance = Instance;
		InstanceItem->DisplayName = Instance == MainInstance
			? FString::Printf(TEXT("Main ABP: %s"), *Instance->GetClass()->GetName())
			: FString::Printf(TEXT("Linked Layer: %s"), *Instance->GetClass()->GetName());

		MeshItem->Children.Add(InstanceItem);
	}
}

void FLAAnimDebuggerViewModel::GatherTargetInstances(TArray<UAnimInstance*>& OutInstances) const
{
	OutInstances.Reset();

	if (!SelectedItem.IsValid())
	{
		return;
	}

	if (SelectedItem->Type == ELAAnimDebuggerTreeItemType::AnimInstance)
	{
		if (UAnimInstance* Instance = SelectedItem->AnimInstance.Get())
		{
			OutInstances.Add(Instance);
		}
		return;
	}

	TArray<USkeletalMeshComponent*> Meshes;
	if (SelectedItem->Type == ELAAnimDebuggerTreeItemType::Mesh)
	{
		if (USkeletalMeshComponent* Mesh = SelectedItem->Mesh.Get())
		{
			Meshes.Add(Mesh);
		}
	}
	else if (AActor* Actor = SelectedItem->Actor.Get())
	{
		Actor->GetComponents(Meshes);
	}

	TSet<UAnimInstance*> Seen;
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		UAnimInstance* MainInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
		if (!MainInstance)
		{
			continue;
		}

		TArray<UAnimInstance*> Instances;
		Instances.Add(MainInstance);
		const USkeletalMeshComponent* ConstMesh = Mesh;
		Instances.Append(ConstMesh->GetLinkedAnimInstances());

		for (UAnimInstance* Instance : Instances)
		{
			if (Instance && !Seen.Contains(Instance))
			{
				Seen.Add(Instance);
				OutInstances.Add(Instance);
			}
		}
	}
}

void FLAAnimDebuggerViewModel::GatherPropertiesForInstance(UAnimInstance* Instance, TArray<FLAAnimDebuggerPropertyItem>& OutProperties) const
{
	if (!Instance)
	{
		return;
	}

	const FString InstanceName = Instance->GetClass()->GetName();

	for (TFieldIterator<FProperty> It(Instance->GetClass()); It; ++It)
	{
		FProperty* Property = *It;
		if (!ShouldDisplayProperty(Property))
		{
			continue;
		}

		FLAAnimDebuggerPropertyItem Item;
		Item.InstanceName = InstanceName;
		Item.Category = Property->GetMetaData(TEXT("Category"));
		if (Item.Category.IsEmpty())
		{
			Item.Category = TEXT("Default");
		}
		Item.PropertyName = Property->GetName();
		Item.Key = FString::Printf(TEXT("%s:%s"), *Instance->GetPathName(), *Property->GetName());
		Item.ValueText = GetPropertyValueText(Property, Instance);
		Item.TypeColor = GetPropertyTypeColor(Property);
		Item.bIsRecorded = IsRecorded(Item.Key);

		if (!SearchText.IsEmpty())
		{
			const bool bMatchesName = Item.PropertyName.Contains(SearchText, ESearchCase::IgnoreCase);
			const bool bMatchesCategory = Item.Category.Contains(SearchText, ESearchCase::IgnoreCase);
			if (!bMatchesName && !bMatchesCategory)
			{
				continue;
			}
		}

		OutProperties.Add(MoveTemp(Item));
	}
}

FString FLAAnimDebuggerViewModel::GetPropertyValueText(FProperty* Property, UObject* Container) const
{
	if (!Property || !Container)
	{
		return TEXT("<unreadable>");
	}

	if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		return FString::Printf(TEXT("%.3f"), FloatProperty->GetPropertyValue_InContainer(Container));
	}

	if (const FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
	{
		return FString::Printf(TEXT("%.3f"), DoubleProperty->GetPropertyValue_InContainer(Container));
	}

	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue_InContainer(Container);
		return ObjectValue ? ObjectValue->GetName() : TEXT("None");
	}

	FString ValueText;
	Property->ExportText_InContainer(0, ValueText, Container, Container, Container, PPF_None);

	constexpr int32 MaxValueLength = 160;
	if (ValueText.Len() > MaxValueLength)
	{
		ValueText.LeftInline(MaxValueLength);
		ValueText += TEXT("...");
	}

	return ValueText.IsEmpty() ? TEXT("<empty>") : ValueText;
}

FLinearColor FLAAnimDebuggerViewModel::GetPropertyTypeColor(FProperty* Property) const
{
	if (CastField<FBoolProperty>(Property)) return FLinearColor(0.90f, 0.10f, 0.10f);
	if (CastField<FIntProperty>(Property) || CastField<FInt64Property>(Property) || CastField<FByteProperty>(Property)) return FLinearColor(0.00f, 0.70f, 0.80f);
	if (CastField<FFloatProperty>(Property) || CastField<FDoubleProperty>(Property)) return FLinearColor(0.30f, 0.90f, 0.30f);
	if (CastField<FEnumProperty>(Property)) return FLinearColor(0.60f, 0.25f, 1.00f);
	if (CastField<FNameProperty>(Property) || CastField<FStrProperty>(Property) || CastField<FTextProperty>(Property)) return FLinearColor(1.00f, 0.25f, 0.70f);
	if (CastField<FObjectPropertyBase>(Property) || CastField<FClassProperty>(Property)) return FLinearColor(0.10f, 0.35f, 1.00f);

	if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		if (StructProperty->Struct == TBaseStructure<FVector>::Get()) return FLinearColor(0.70f, 1.00f, 0.20f);
		if (StructProperty->Struct == TBaseStructure<FRotator>::Get()) return FLinearColor(0.90f, 0.25f, 1.00f);
		if (StructProperty->Struct == TBaseStructure<FTransform>::Get()) return FLinearColor(0.95f, 0.60f, 0.20f);
	}

	return FLinearColor::Gray;
}

bool FLAAnimDebuggerViewModel::ShouldDisplayProperty(FProperty* Property) const
{
	if (!Property)
	{
		return false;
	}

	if (CastField<FDelegateProperty>(Property) || CastField<FMulticastDelegateProperty>(Property))
	{
		return false;
	}

	if (CastField<FArrayProperty>(Property) || CastField<FMapProperty>(Property) || CastField<FSetProperty>(Property))
	{
		return false;
	}

	return Property->HasAnyPropertyFlags(CPF_BlueprintVisible | CPF_Edit);
}

void FLAAnimDebuggerViewModel::UpdateLogs(double CurrentTimeSeconds, const TArray<FLAAnimDebuggerPropertyItem>& NewProperties)
{
	for (const FLAAnimDebuggerPropertyItem& Property : NewProperties)
	{
		const FString* OldValue = LastValues.Find(Property.Key);
		if (OldValue && *OldValue != Property.ValueText && RecordedKeys.Contains(Property.Key))
		{
			TSharedPtr<FLAAnimDebuggerLogItem> LogItem = MakeShared<FLAAnimDebuggerLogItem>();
			LogItem->TimeSeconds = CurrentTimeSeconds;
			LogItem->InstanceName = Property.InstanceName;
			LogItem->Category = Property.Category;
			LogItem->PropertyName = Property.PropertyName;
			LogItem->OldValue = *OldValue;
			LogItem->NewValue = Property.ValueText;
			Logs.Add(LogItem);

			while (Logs.Num() > MaxLogCount)
			{
				Logs.RemoveAt(0);
			}
		}

		LastValues.Add(Property.Key, Property.ValueText);
	}
}

void FLAAnimDebuggerViewModel::LoadSettings()
{
	if (!GConfig)
	{
		return;
	}

	GConfig->GetFloat(LAAnimDebugger::SettingsSection, TEXT("RefreshInterval"), RefreshInterval, GEditorPerProjectIni);
	GConfig->GetBool(LAAnimDebugger::SettingsSection, TEXT("FollowEditorSelection"), bFollowEditorSelection, GEditorPerProjectIni);

	TArray<FString> SavedRecordedKeys;
	GConfig->GetArray(LAAnimDebugger::SettingsSection, TEXT("RecordedKeys"), SavedRecordedKeys, GEditorPerProjectIni);
	RecordedKeys.Reset();
	for (const FString& Key : SavedRecordedKeys)
	{
		RecordedKeys.Add(Key);
	}
}

void FLAAnimDebuggerViewModel::SaveSettings() const
{
	if (!GConfig)
	{
		return;
	}

	GConfig->SetFloat(LAAnimDebugger::SettingsSection, TEXT("RefreshInterval"), RefreshInterval, GEditorPerProjectIni);
	GConfig->SetBool(LAAnimDebugger::SettingsSection, TEXT("FollowEditorSelection"), bFollowEditorSelection, GEditorPerProjectIni);

	TArray<FString> SavedRecordedKeys = RecordedKeys.Array();
	GConfig->SetArray(LAAnimDebugger::SettingsSection, TEXT("RecordedKeys"), SavedRecordedKeys, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}
