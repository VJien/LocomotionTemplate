#include "Core/LTGameMode.h"
#include "Character/LTCharacter.h"
#include "Character/LTPlayerController.h"
#include "Character/LTHUD.h"
#include "Core/LTPlayerState.h"

ALTGameMode::ALTGameMode()
{
	DefaultPawnClass = ALTCharacter::StaticClass();
	PlayerControllerClass = ALTPlayerController::StaticClass();
	HUDClass = ALTHUD::StaticClass();
	PlayerStateClass = ALTPlayerState::StaticClass();
}
