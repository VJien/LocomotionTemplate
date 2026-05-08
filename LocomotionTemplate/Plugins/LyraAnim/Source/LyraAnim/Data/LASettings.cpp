#include "LASettings.h"

ULASettings::ULASettings()
{
}

void ULASettings::SetToggleSettings(const FLAAnimToggleSettings& InSettings)
{
	FLAAnimToggleSettings Old = ToggleSettings;
	ToggleSettings = InSettings;
	DiffAndBroadcast(Old, ToggleSettings);
}

void ULASettings::DiffAndBroadcast(const FLAAnimToggleSettings& Old, const FLAAnimToggleSettings& New)
{
#define LA_DIFF_BOOL(Prop) \
	if (Old.Prop != New.Prop) OnToggleChanged.Broadcast(FName(TEXT(#Prop)), New.Prop);

	LA_DIFF_BOOL(bUseBlendSpaceLoop);
	LA_DIFF_BOOL(bEnableStart);
	LA_DIFF_BOOL(bEnableStartDM);
	LA_DIFF_BOOL(bEnableStartStrideWarping);
	LA_DIFF_BOOL(bEnableStop);
	LA_DIFF_BOOL(bEnableStopDM);
	LA_DIFF_BOOL(bEnableStopStrideWarping);
	LA_DIFF_BOOL(bEnableCycleDM);
	LA_DIFF_BOOL(bEnableCycleStrideWarping);
	LA_DIFF_BOOL(bEnablePivot);
	LA_DIFF_BOOL(bEnablePivotStrideWarping);
	LA_DIFF_BOOL(bEnableAimOffset);
	LA_DIFF_BOOL(bEnableTurnInPlace);
	LA_DIFF_BOOL(bEnableFootIK);
	LA_DIFF_BOOL(bEnableLean);

#undef LA_DIFF_BOOL
}
