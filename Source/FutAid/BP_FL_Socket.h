// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IWebSocket.h"
#include "BP_FL_Socket.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnected);

/**
 * 
 */
UCLASS()
class FUTAID_API UBP_FL_Socket : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	static TSharedPtr<IWebSocket> s_socket;

	UFUNCTION(BlueprintCallable, Category = "Haptics|WebSocket")
	static void ConnectToArduino(FString ServerURL = TEXT("ws://192.168.4.1:80/ws"));

	UFUNCTION(BlueprintCallable, Category = "Haptics|WebSocket")
	static void DisconnectFromArduino();

	UFUNCTION(BlueprintCallable, Category = "Haptics|WebSocket")
	static void SendHapticCommand(FString Command);

	UPROPERTY(BlueprintAssignable, Category = "Haptics|WebSocket")
	FOnMessageReceived OnMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Haptics|WebSocket")
	FOnConnected OnSocketConnected;
};
