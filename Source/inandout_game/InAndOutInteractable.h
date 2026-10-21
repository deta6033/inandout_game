#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InAndOutInteractable.generated.h"

UINTERFACE(BlueprintType)
class INANDOUT_GAME_API UInAndOutInteractable : public UInterface
{
    GENERATED_BODY()
};

class INANDOUT_GAME_API IInAndOutInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    FText GetInteractionText() const;
};
