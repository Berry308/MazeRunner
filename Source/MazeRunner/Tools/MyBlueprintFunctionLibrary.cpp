// Fill out your copyright notice in the Description page of Project Settings.


#include "Tools/MyBlueprintFunctionLibrary.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"

FVector UMyBlueprintFunctionLibrary::GetProjectedNavLocation(AActor* TargetActor, float SearchRadius)
{
    if (!TargetActor)
        return FVector::ZeroVector;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(TargetActor->GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavSystem not found! Ensure NavMesh is built."));
        return TargetActor->GetActorLocation();
    }

    FNavLocation ProjectedLocation;
    bool bSuccess = NavSys->ProjectPointToNavigation(
        TargetActor->GetActorLocation(),
        ProjectedLocation,
        FVector(SearchRadius)
    );

    if (bSuccess)
    {
        // 打印到输出日志，方便直接复制坐标
        UE_LOG(LogTemp, Warning, TEXT("Precise Nav Location for %s: %s"), *TargetActor->GetName(), *ProjectedLocation.Location.ToString());
        return ProjectedLocation.Location;
    }

    return bSuccess ? ProjectedLocation.Location : TargetActor->GetActorLocation();
}