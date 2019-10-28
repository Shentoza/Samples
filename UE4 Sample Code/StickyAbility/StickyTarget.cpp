// Fill out your copyright notice in the Description page of Project Settings.

#include "StickyTarget.h"
#include "StickyLight.h"
#include "Components/ActorComponent.h"


// Sets default values for this component's properties
UStickyTarget::UStickyTarget()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetTargetActive(true);
	// ...
}


// Called when the game starts
void UStickyTarget::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UStickyTarget::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStickyTarget::OnAttach(UStickyLight* attachingComponent) {
	connectedLight = attachingComponent;
	attachingComponent->OnPullDelegate.AddDynamic(this, &UStickyTarget::OnPull);
	attachingComponent->OnPushDelegate.AddDynamic(this, &UStickyTarget::OnPush);
	attachingComponent->OnAttachStayDelegate.AddDynamic(this, &UStickyTarget::OnAttachStay);
	if (OnAttachDetachDelegate.IsBound())
		OnAttachDetachDelegate.Broadcast(true);
}

void UStickyTarget::OnDetach() {
	connectedLight->OnPullDelegate.RemoveDynamic(this, &UStickyTarget::OnPull);
	connectedLight->OnPushDelegate.RemoveDynamic(this, &UStickyTarget::OnPush);
	connectedLight->OnAttachStayDelegate.RemoveDynamic(this, &UStickyTarget::OnAttachStay);

	connectedLight = nullptr;
	if (OnAttachDetachDelegate.IsBound()) {
		OnAttachDetachDelegate.Broadcast(false);
	}
}

void UStickyTarget::OnAttachStay(const float DeltaTime) {
	ReceiveOnAttachedStay(DeltaTime);
}

void UStickyTarget::OnPush(const float DeltaTime) {
	ReceiveOnPush(DeltaTime);
	if (OnPushDelegate.IsBound())
		OnPushDelegate.Broadcast();
}

void UStickyTarget::OnPull(const float DeltaTime) {
	ReceiveOnPull(DeltaTime);
	if (OnPullDelegate.IsBound())
		OnPullDelegate.Broadcast();
}

void UStickyTarget::OnTargeted(const bool targetActive)
{
	if (OnTargetUnTargetDelegate.IsBound()) {
		OnTargetUnTargetDelegate.Broadcast(targetActive);
	}
}

bool UStickyTarget::IsAttached() const
{
	return connectedLight->IsValidLowLevel();
}