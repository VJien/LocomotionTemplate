#include "Core/LTProjectSettings.h"

FName ULTProjectSettings::GetCategoryName() const
{
	return FName("Game");
}

FName ULTProjectSettings::GetSectionName() const
{
	return FName("Locomotion Template");
}

const ULTProjectSettings* ULTProjectSettings::Get()
{
	return GetDefault<ULTProjectSettings>();
}
