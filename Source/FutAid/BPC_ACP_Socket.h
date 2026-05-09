
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IWebSocket.h"
#include "BPC_ACP_Socket.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnHapticDataReceived, float, GyroX, float, GyroY, float, GyroZ, int32, BtnFront, int32, BtnBack);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FUTAID_API UBPC_ACP_Socket : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBPC_ACP_Socket();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void Connect(FString ServerURL = TEXT("ws://192.168.4.1:80/ws"));

	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void SendCommand(FString Command);

	UFUNCTION(BlueprintCallable, Category = "Haptics")
	bool IsConnected() const;

	UPROPERTY(BlueprintAssignable, Category = "Haptics")
	FOnHapticDataReceived OnDataReceived;

protected:
	virtual void BeginPlay() override;

public:
	TSharedPtr<IWebSocket> m_socket;

	float LastGyroX, LastGyroY, LastGyroZ;
	int32 LastBtnFront, LastBtnBack;

	TAtomic<bool> bHasNewData;

	FCriticalSection DataMutex;

	float ReconnectTimer;
	float ReconnectInterval;
	FString SavedURL;
	bool bShouldReconnect;

	void SetupSocketCallbacks();
};
