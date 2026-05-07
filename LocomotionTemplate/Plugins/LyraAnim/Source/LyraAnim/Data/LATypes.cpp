// Fill out your copyright notice in the Description page of Project Settings.


#include "LATypes.h"

void FLAAnimLinkProfile::Link(USkeletalMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		if (OverrideAnimInstance)
		{
			MeshComponent->SetAnimInstanceClass(OverrideAnimInstance);
		}
		for (auto&& Layer : ClassLinkedAnimInstances)
		{
			MeshComponent->LinkAnimClassLayers(Layer);
		}
		for (auto&& Layer : TagLinkedAnimInstances)
		{
			MeshComponent->LinkAnimGraphByTag(Layer.Key, Layer.Value);
		}
	}
}

void FLAAnimLinkProfile::Unlink(USkeletalMeshComponent* MeshComponent)
{
	if (MeshComponent)
	{
		for (auto&& Layer : ClassLinkedAnimInstances)
		{
			MeshComponent->UnlinkAnimClassLayers(Layer);
		}
		for (auto&& Layer : TagLinkedAnimInstances)
		{
			MeshComponent->UnlinkAnimClassLayers(Layer.Value);
		}
	}
}

void FLAAnimLinkProfile::Reset()
{
	OverrideAnimInstance = nullptr;
	ClassLinkedAnimInstances.Empty();
	TagLinkedAnimInstances.Empty();
}
