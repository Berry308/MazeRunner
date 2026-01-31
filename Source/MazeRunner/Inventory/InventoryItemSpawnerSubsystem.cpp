// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryItemSpawnerSubsystem.h"

#include "Inventory/MaruInventoryItemInstance.h"
#include "MazeRunnerLogChannels.h"
#include "Equipment/MaruQuickBarComponent.h"

UMaruInventoryItemInstance* UInventoryItemSpawnerSubsystem::CreateItemInstanceFromDefinition(TSubclassOf<UMaruInventoryItemDefinition> ItemDef, const TArray<FGameplayTag>& TagToInit)
{
    UMaruInventoryItemInstance* ItemInstance = NewObject<UMaruInventoryItemInstance>();

    if (ItemDef == nullptr)
    {
        UE_LOG(LogMR, Warning, TEXT("CreateItemInstanceFromDefinition has failed!"));
        return ItemInstance;
    }

    ItemInstance->SetItemDef(ItemDef);

    if (!TagToInit.IsEmpty())
    {
        for (FGameplayTag Tag : TagToInit)
        {
            ItemInstance->AddStatTagStack(Tag, 1);
        }
    }

    return ItemInstance;
}

void UInventoryItemSpawnerSubsystem::GiveToPlayerQuickBar(APlayerController* Player,UMaruInventoryItemInstance* Item)
{
    if (UMaruQuickBarComponent* QuickBar = Player->FindComponentByClass<UMaruQuickBarComponent>())
    {
        int32 ItemSlot = QuickBar->GetNextFreeItemSlot();
        QuickBar->AddItemToSlot(ItemSlot, Item);
        QuickBar->SetActiveSlotIndex(ItemSlot);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UInventoryItemSpawnerSubsystem::GiveToPlayerQuickBar Give %s To %s have failed"), *Player->GetName(), *Item->GetItemDef()->GetName());
    }
}

//为当前世界的所有PlayerController的QuickBar添加Item
void UInventoryItemSpawnerSubsystem::GiveToAllPlayerQuickBar(UMaruInventoryItemInstance* Item)
{
    UGameInstance* GameInstance = GetGameInstance();
    UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;

    if (World)
    {
        /*for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
        {
            ULocalPlayer* LP = *LocalPlayerIterator;
            APlayerController* PC = LP->GetPlayerController(World);
            if (PC)
            {
                GiveToPlayerQuickBar(PC, Item);
            }
        }*/
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (PC)
            {
                GiveToPlayerQuickBar(PC, Item);
            }
        }
    }
}
