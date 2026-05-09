// Fill out your copyright notice in the Description page of Project Settings.

#include "BP_FL_Socket.h"
#include "WebSocketsModule.h"

TSharedPtr<IWebSocket> UBP_FL_Socket::s_socket = nullptr;

void UBP_FL_Socket::ConnectToArduino(FString ServerURL)
{
	FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));

	s_socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, TEXT("ws"));

	s_socket->OnConnected().AddLambda([]() {
		UE_LOG(LogTemp, Warning, TEXT("Conectado exitosamente al Wemos D1 R1"));
	});

	s_socket->OnConnectionError().AddLambda([](const FString& Error) {
		UE_LOG(LogTemp, Error, TEXT("Error de conexión: %s"), *Error);
	});

	s_socket->OnMessage().AddLambda([](const FString& MessageString) {
		UE_LOG(LogTemp, Log, TEXT("Datos del Arduino: %s"), *MessageString);
	});

	s_socket->Connect();
}

void UBP_FL_Socket::DisconnectFromArduino()
{
	if (s_socket.IsValid() && s_socket->IsConnected()) {
		s_socket->Close();
		UE_LOG(LogTemp, Warning, TEXT("Socket cerrado"));
	}
}

void UBP_FL_Socket::SendHapticCommand(FString Command)
{
	if (s_socket.IsValid() && s_socket->IsConnected()) {
		s_socket->Send(Command);
	}
}