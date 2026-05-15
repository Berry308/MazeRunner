// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MaruUISubsystem.h"
#include "MaruHUD.h"

void UMaruUISubsystem::DeliverWidgetToAllPlayer(EWidgetLayer WidgetLayer, UActivatableWidget* Widget)
{
    //获取所有PlayerController的HUD
    UWorld* World = GetWorld();
    if (!World) return;

    // 遍历世界中所有的 PlayerController
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC)
        {
            //获取玩家HUD
            AMaruHUD* MaruHUD = Cast<AMaruHUD>(PC->GetHUD());
            if (MaruHUD)
            {
                //传递Widget
                MaruHUD->AddWidgetToLayer(WidgetLayer,Widget);
            }
        }
    }
}