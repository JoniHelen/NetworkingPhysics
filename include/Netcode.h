#pragma once

namespace NetPhysics {

	inline std::vector<std::future<int>> SendOperations;

	inline std::vector<Socket> Clients;

	int WSAInit();

	int GetAddressInfo(AddressInfo** info, const wchar_t* address = L"127.0.0.1", const wchar_t* port = L"56789");

	int SetSocketBlockingMode(bool mode);

	int BindSocketToAddress(Socket s, const AddressInfo* info);

	int ConnectToAddress(Socket s, const AddressInfo* info);

	int ListenWithSocket(Socket s);

	bool SocketIsValid(Socket s);

	bool FlagNotSet(const RunningFlag& flag);

	Socket TryAccept(Socket listener);

	Socket CreateStreamSocket();

	int SendDataToClient(Socket client);

	int BroadcastTriangleData();

	void TimedSend(long long ns, const RunningFlag& flag);

	int ListenForClients(const RunningFlag& running);

	int ConnectToServer(const RunningFlag& running);
}