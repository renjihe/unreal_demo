// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AISense_Attract.h"

#include "Perception/AIPerceptionSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "AISenseConfig_Attract.h"
#include "AISenseEvent_Attract.h"

/*#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerTypes.h"
#include "GameplayDebuggerCategory.h"
#endif*/

//----------------------------------------------------------------------//
// UAISenseConfig_Hearing
//----------------------------------------------------------------------//

UAISenseConfig_Attract::UAISenseConfig_Attract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), AttractRange(3000.f)
{
	DebugColor = FColor::Yellow;
}

TSubclassOf<UAISense> UAISenseConfig_Attract::GetSenseImplementation() const
{
	return UAISense_Attract::StaticClass();
}

/*#if WITH_GAMEPLAY_DEBUGGER
static FString DescribeColorHelper(const FColor& Color)
{
	int32 MaxColors = GColorList.GetColorsNum();
	for (int32 Idx = 0; Idx < MaxColors; Idx++)
	{
		if (Color == GColorList.GetFColorByIndex(Idx))
		{
			return GColorList.GetColorNameByIndex(Idx);
		}
	}

	return FString(TEXT("color"));
}

void UAISenseConfig_Attract::DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent, FGameplayDebuggerCategory* DebuggerCategory) const
{
	if (PerceptionComponent == nullptr || DebuggerCategory == nullptr)
	{
		return;
	}

	FColor AttractRangeColor = FColor::Yellow;

	// don't call Super implementation on purpose, replace color description line
	DebuggerCategory->AddTextLine(
		FString::Printf(TEXT("%s: {%s}%s {white}range:{%s}%s"), *GetSenseName(),
			*GetDebugColor().ToString(), *DescribeColorHelper(GetDebugColor()),
			*AttractRangeColor.ToString(), *DescribeColorHelper(AttractRangeColor))
	);

	const AActor* BodyActor = PerceptionComponent->GetBodyActor();
	if (BodyActor != nullptr)
	{
		FVector OwnerLocation = BodyActor->GetActorLocation();

		DebuggerCategory->AddShape(FGameplayDebuggerShape::MakeCylinder(OwnerLocation, AttractRange, 25.0f, AttractRangeColor));
	}
}
#endif // WITH_GAMEPLAY_DEBUGGER*/

//----------------------------------------------------------------------//
// UAISenseEvent_Attract
//----------------------------------------------------------------------//
UAISenseEvent_Attract::UAISenseEvent_Attract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FAISenseID UAISenseEvent_Attract::GetSenseID() const
{
	return UAISense::GetSenseID<UAISense_Attract>();
}


//----------------------------------------------------------------------//
// FAIAttractEvent
//----------------------------------------------------------------------//
FAIAttractEvent::FAIAttractEvent()
	: AttractLocation(FAISystem::InvalidLocation), MaxRange(0.f)
	, Instigator(nullptr), Tag(NAME_None), TeamIdentifier(FGenericTeamId::NoTeam)
{
}

FAIAttractEvent::FAIAttractEvent(AActor* InInstigator, const FVector& InAttractLocation, float InMaxRange, FName InTag)
	: AttractLocation(InAttractLocation), MaxRange(InMaxRange)
	, Instigator(InInstigator), Tag(InTag), TeamIdentifier(FGenericTeamId::NoTeam)
{
	Compile();
}

void FAIAttractEvent::Compile()
{
	TeamIdentifier = FGenericTeamId::GetTeamIdentifier(Instigator);
	if (FAISystem::IsValidLocation(AttractLocation) == false && Instigator != nullptr)
	{
		AttractLocation = Instigator->GetActorLocation();
	}
}

//----------------------------------------------------------------------//
// FDigestedAttractProperties
//----------------------------------------------------------------------//
UAISense_Attract::FDigestedAttractProperties::FDigestedAttractProperties(const UAISenseConfig_Attract& SenseConfig)
{
	AttractRange = SenseConfig.AttractRange;
	AffiliationFlags = SenseConfig.DetectionByAffiliation.GetAsFlags();
}

UAISense_Attract::FDigestedAttractProperties::FDigestedAttractProperties()
	: AttractRange(-1.f), AffiliationFlags(-1)
{

}

//----------------------------------------------------------------------//
// UAISense_Attract
//----------------------------------------------------------------------//
UAISense_Attract::UAISense_Attract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		OnNewListenerDelegate.BindUObject(this, &UAISense_Attract::OnNewListenerImpl);
		OnListenerUpdateDelegate.BindUObject(this, &UAISense_Attract::OnListenerUpdateImpl);
		OnListenerRemovedDelegate.BindUObject(this, &UAISense_Attract::OnListenerRemovedImpl);
	}
}

void UAISense_Attract::ReportAttractEvent(UObject* WorldContext, FVector AttractLocation, AActor* Instigator, float MaxRange, FName Tag)
{
	UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(WorldContext);
	if (PerceptionSystem)
	{
		FAIAttractEvent Event(Instigator, AttractLocation, MaxRange, Tag);
		PerceptionSystem->OnEvent(Event);
	}
}

void UAISense_Attract::OnNewListenerImpl(const FPerceptionListener& NewListener)
{
	check(NewListener.Listener.IsValid());
	const UAISenseConfig_Attract* SenseConfig = Cast<const UAISenseConfig_Attract>(NewListener.Listener->GetSenseConfig(GetSenseID()));
	check(SenseConfig);
	const FDigestedAttractProperties PropertyDigest(*SenseConfig);
	DigestedProperties.Add(NewListener.GetListenerID(), PropertyDigest);
}

void UAISense_Attract::OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener)
{
	// @todo add updating code here
	const FPerceptionListenerID ListenerID = UpdatedListener.GetListenerID();

	if (UpdatedListener.HasSense(GetSenseID()))
	{
		const UAISenseConfig_Attract* SenseConfig = Cast<const UAISenseConfig_Attract>(UpdatedListener.Listener->GetSenseConfig(GetSenseID()));
		check(SenseConfig);
		FDigestedAttractProperties& PropertiesDigest = DigestedProperties.FindOrAdd(ListenerID);
		PropertiesDigest = FDigestedAttractProperties(*SenseConfig);
	}
	else
	{
		DigestedProperties.Remove(ListenerID);
	}
}

void UAISense_Attract::OnListenerRemovedImpl(const FPerceptionListener& UpdatedListener)
{
	DigestedProperties.FindAndRemoveChecked(UpdatedListener.GetListenerID());
}

float UAISense_Attract::Update()
{
	AIPerception::FListenerMap& ListenersMap = *GetListeners();
	//UAIPerceptionSystem* PerseptionSys = GetPerceptionSystem();

	for (AIPerception::FListenerMap::TIterator ListenerIt(ListenersMap); ListenerIt; ++ListenerIt)
	{
		FPerceptionListener& Listener = ListenerIt->Value;

		if (Listener.HasSense(GetSenseID()) == false)
		{
			// skip listeners not interested in this sense
			continue;
		}

		const FDigestedAttractProperties& PropDigest = DigestedProperties[Listener.GetListenerID()];

		for (int32 EventIndex = 0; EventIndex < AttractEvents.Num(); ++EventIndex)
		{
			const FAIAttractEvent& Event = AttractEvents[EventIndex];
			const float DistToAttract = FVector::Distance(Event.AttractLocation, Listener.CachedLocation);

			// Limit by listener max range
			if (DistToAttract > PropDigest.AttractRange)
			{
				continue;
			}
			// Limit by event max range
			else if (Event.MaxRange > 0.f && DistToAttract > Event.MaxRange)
			{
				continue;
			}

			if (FAISenseAffiliationFilter::ShouldSenseTeam(Listener.TeamIdentifier, Event.TeamIdentifier, PropDigest.AffiliationFlags) == false)
			{
				continue;
			}
					
			Listener.RegisterStimulus(Event.Instigator, FAIStimulus(*this, DistToAttract, Event.AttractLocation, Listener.CachedLocation, FAIStimulus::SensingSucceeded, Event.Tag));
		}
	}

	AttractEvents.Reset();

	// return decides when next tick is going to happen
	return SuspendNextUpdate;
}

void UAISense_Attract::RegisterEvent(const FAIAttractEvent& Event)
{
	AttractEvents.Add(Event);

	RequestImmediateUpdate();
}

void UAISense_Attract::RegisterWrappedEvent(UAISenseEvent& PerceptionEvent)
{
	UAISenseEvent_Attract* AttractEvent = Cast<UAISenseEvent_Attract>(&PerceptionEvent);
	ensure(AttractEvent);
	if (AttractEvent)
	{
		RegisterEvent(AttractEvent->GetAttractEvent());
	}
}


