#include "SvGame.h"
#include "BTService_BlueprintBaseSV.h"

void UBTService_BlueprintBaseSV::SetOwner(AActor* owner)
{
	Super::SetOwner(owner);

	BtAIOwner = Cast<AAIController>(owner);
}
