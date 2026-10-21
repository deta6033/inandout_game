#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InAndOutGameMode.generated.h"

class ASonCharacter;
class AMomCharacter;

UCLASS()
class INANDOUT_GAME_API AInAndOutGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AInAndOutGameMode();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ASonCharacter> SonCharacter;

    UPROPERTY()
    TObjectPtr<AMomCharacter> MomCharacter;

    FTimerHandle BindTimerHandle;
    bool bMatchEnded = false;

    void TryBindCombatants();
    void FinishMatch(bool bVictory);

    UFUNCTION()
    void HandleSonDeath(AActor* DamageCauser);

    UFUNCTION()
    void HandleMomDeath(AActor* DamageCauser);
};
