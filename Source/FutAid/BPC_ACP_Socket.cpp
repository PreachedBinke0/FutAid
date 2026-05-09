
#include "BPC_ACP_Socket.h"
#include "WebSocketsModule.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UBPC_ACP_Socket::UBPC_ACP_Socket() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bHasNewData = false;
	ReconnectTimer = 0.f;
	ReconnectInterval = 3.f;
	bShouldReconnect = false;
	LastGyroX = LastGyroY = LastGyroZ = 0.f;
	LastBtnFront = LastBtnBack = 0;
}

void UBPC_ACP_Socket::BeginPlay()
{
	Super::BeginPlay();
}

void UBPC_ACP_Socket::SetupSocketCallbacks()
{
	m_socket->OnConnected().AddLambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("[Haptic] Socket connected to %s"), *SavedURL);
			ReconnectTimer = 0.f;
		});

	m_socket->OnMessage().AddLambda([this](const FString& MessageString)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageString);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				float gX = (float)JsonObject->GetNumberField(TEXT("gyro_ax"));
				float gY = (float)JsonObject->GetNumberField(TEXT("gyro_ay"));
				float gZ = (float)JsonObject->GetNumberField(TEXT("gyro_az"));
				int32 bFront = JsonObject->GetIntegerField(TEXT("btnfront"));
				int32 bBack = JsonObject->GetIntegerField(TEXT("btnback"));

				FScopeLock Lock(&DataMutex);
				LastGyroX = gX;
				LastGyroY = gY;
				LastGyroZ = gZ;
				LastBtnFront = bFront;
				LastBtnBack = bBack;
				bHasNewData = true; 
			} else {
				UE_LOG(LogTemp, Error, TEXT("[Haptic] JSON malformado: %s"), *MessageString);
			}
		});

	m_socket->OnConnectionError().AddLambda([this](const FString& Error)
		{
			UE_LOG(LogTemp, Error, TEXT("[Haptic] Error: %s"), *Error);
			bShouldReconnect = true;
		});

	m_socket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Haptic] Connection Closed (%d): %s | Clear: %s"),
				StatusCode, *Reason, bWasClean ? TEXT("si") : TEXT("no"));

			bShouldReconnect = true;
		});
}

void UBPC_ACP_Socket::Connect(FString ServerURL) {
	SavedURL = ServerURL;
	bShouldReconnect = false;

	FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	m_socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, TEXT("ws"));

	SetupSocketCallbacks();
	m_socket->Connect();
}

void UBPC_ACP_Socket::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldReconnect)
	{
		ReconnectTimer += DeltaTime;
		if (ReconnectTimer >= ReconnectInterval)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Haptic] Intentando reconectar..."));
			ReconnectTimer = 0.f;
			bShouldReconnect = false;
			Connect(SavedURL);
		}
	}

	if (bHasNewData)
	{
		float gX, gY, gZ;
		int32 bF, bB;

		{
			FScopeLock Lock(&DataMutex);
			gX = LastGyroX; gY = LastGyroY; gZ = LastGyroZ;
			bF = LastBtnFront; bB = LastBtnBack;
			bHasNewData = false;
		}

		OnDataReceived.Broadcast(gX, gY, gZ, bF, bB);
	}
}

bool UBPC_ACP_Socket::IsConnected() const
{
	return m_socket.IsValid() && m_socket->IsConnected();
}

void UBPC_ACP_Socket::SendCommand(FString Command) {
	if (m_socket.IsValid() && m_socket->IsConnected()) {
		m_socket->Send(Command);
	}
}

void UBPC_ACP_Socket::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	bShouldReconnect = false;
	if (m_socket.IsValid() && m_socket->IsConnected()) {
		m_socket->Close();
	}
	Super::EndPlay(EndPlayReason);
}
