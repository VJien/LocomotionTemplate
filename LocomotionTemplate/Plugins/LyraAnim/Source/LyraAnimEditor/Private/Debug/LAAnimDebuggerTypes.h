#pragma once

#include "CoreMinimal.h"

class AActor;
class UAnimInstance;
class USkeletalMeshComponent;

enum class ELAAnimDebuggerTreeItemType : uint8
{
	Actor,
	Mesh,
	AnimInstance
};

struct FLAAnimDebuggerTreeItem : public TSharedFromThis<FLAAnimDebuggerTreeItem>
{
	ELAAnimDebuggerTreeItemType Type = ELAAnimDebuggerTreeItemType::Actor;
	FString DisplayName;
	TWeakObjectPtr<AActor> Actor;
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TArray<TSharedPtr<FLAAnimDebuggerTreeItem>> Children;
};

struct FLAAnimDebuggerPropertyItem
{
	FString Key;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString ValueText;
	FLinearColor TypeColor = FLinearColor::Gray;
	bool bIsRecorded = false;
};

struct FLAAnimDebuggerLogItem
{
	double TimeSeconds = 0.0;
	FString InstanceName;
	FString Category;
	FString PropertyName;
	FString OldValue;
	FString NewValue;
};
