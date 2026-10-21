#include "InAndOutMenuGameMode.h"

#include "InAndOutMenuPlayerController.h"

AInAndOutMenuGameMode::AInAndOutMenuGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = AInAndOutMenuPlayerController::StaticClass();
    HUDClass = nullptr;
    bStartPlayersAsSpectators = true;
}
