#include <pch.h>
#include <Netcode.h>
#include <NetworkingPhysics.h>

namespace NetPhysics {
	int WSAInit() {
		WSAData data; 
		return WSAStartup(MAKEWORD(2, 2), &data);
	}

	int GetAddressInfo(AddressInfo** info, const wchar_t* address, const wchar_t* port) {
		AddressInfo hints {};
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		return GetAddrInfo(address, port, &hints, info);
	}

	int SetSocketBlockingMode(const Socket s, const bool mode) {
		auto val = mode ? 0ul : 1ul, bytes = 0ul;
		return WSAIoctl(s, FIONBIO, &val, sizeof(val), nullptr,
			0, &bytes, nullptr, nullptr);
	}

	int BindSocketToAddress(const Socket s, const AddressInfo* const info) {
		return bind(s, info->ai_addr, info->ai_addrlen);
	}

	int ConnectToAddress(Socket s, const AddressInfo* const info) {
		return connect(s, info->ai_addr, info->ai_addrlen);
	}

	int ListenWithSocket(Socket s) {
		return listen(s, SOMAXCONN);
	}

	bool SocketIsValid(Socket s) {
		return s != INVALID_SOCKET;
	}

	bool FlagNotSet(const RunningFlag& flag) {
		return !flag.test(std::memory_order::relaxed);
	}

	Socket TryAccept(Socket listener) {
		return WSAAccept(listener, nullptr, nullptr, nullptr, 0ull);
	}

	Socket CreateStreamSocket() {
		return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
			nullptr, 0, WSA_FLAG_OVERLAPPED);
	}

	int SendDataToClient(const Socket client) {
		if (WSAInit() != 0) return -1;

		Buffer dataBuf {
			sizeof(TriData), reinterpret_cast<CHAR*>(TriData)
		};

		auto bytesSent = 0ul;

		if (WSASend(client, &dataBuf, 1ul, &bytesSent, 0, nullptr, nullptr) == SOCKET_ERROR) {
			if (int err = WSAGetLastError(); err != WSAEWOULDBLOCK) {
				std::cerr << "Error on SEND: " << err << "\n";
				std::cerr << "Aborting connection on socket " << client << "\n";
				closesocket(client);
				Clients.erase(std::remove(Clients.begin(), Clients.end(), client));
			}
		}

		WSACleanup();
		return 0;
	}

	int BroadcastTriangleData() {
		Lock lock(TriDataMutex);

		SendOperations.clear();

		for (const Socket& client : Clients) {
			SendOperations.push_back(std::async(SendDataToClient, client));
		}

		for (const std::future<int>& send : SendOperations) {
			send.wait();
		}

		return 0;
	}

	void TimedSend(long long ns, const RunningFlag& running) {
		while(FlagNotSet(running)) {
			std::this_thread::sleep_for(std::chrono::seconds(ns));
			BroadcastTriangleData();
		}
	}

	int ListenForClients(const RunningFlag& running) {

		if (WSAInit() != 0) return -1;

		AddressInfo* addressInfo;

		if (GetAddressInfo(&addressInfo) != 0) return -1;

		const Socket ListenSocket = CreateStreamSocket();

		if (ListenSocket == INVALID_SOCKET) {
			FreeAddrInfo(addressInfo); WSACleanup(); return -1;
		}

		if (SetSocketBlockingMode(ListenSocket, false) == SOCKET_ERROR) {
			FreeAddrInfo(addressInfo); closesocket(ListenSocket); WSACleanup(); return -1;
		}

		if (BindSocketToAddress(ListenSocket, addressInfo) == SOCKET_ERROR) {
			FreeAddrInfo(addressInfo); closesocket(ListenSocket); WSACleanup(); return -1;
		}

		if (ListenWithSocket(ListenSocket) == SOCKET_ERROR) {
			FreeAddrInfo(addressInfo); closesocket(ListenSocket); WSACleanup(); return -1;
		}

		FreeAddrInfo(addressInfo);

		Socket ClientSocket = INVALID_SOCKET;

		while (FlagNotSet(running)) {
			ClientSocket = TryAccept(ListenSocket);

			if (SocketIsValid(ClientSocket)) {
				std::cout << "Client connected!\n";
				Clients.push_back(ClientSocket);
			}
		}

		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	int ConnectToServer(const RunningFlag& running) {

		if (WSAInit() != 0) return -1;

		AddressInfo* addressInfo;

		if (GetAddressInfo(&addressInfo) != 0) return -1;

		const SOCKET ConnectSocket = CreateStreamSocket();

		if (ConnectSocket == INVALID_SOCKET) {
			FreeAddrInfo(addressInfo); WSACleanup(); return -1;
		}

		if (ConnectToAddress(ConnectSocket, addressInfo) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			FreeAddrInfo(addressInfo); closesocket(ConnectSocket); WSACleanup(); return -1;
		}

		if (SetSocketBlockingMode(ConnectSocket, false) == SOCKET_ERROR) {
			FreeAddrInfo(addressInfo); closesocket(ConnectSocket); WSACleanup(); return -1;
		}

		FreeAddrInfo(addressInfo);

		if (ConnectSocket == INVALID_SOCKET) {
			WSACleanup(); return -1;
		}

		Buffer recvBuffer {
			sizeof(TriData), reinterpret_cast<CHAR*>(TriData)
		};

		auto bytesRecvd = 0ul;

		auto flags = 0ul;

		while (FlagNotSet(running)) {
			const int rc = WSARecv(ConnectSocket, &recvBuffer, 1, &bytesRecvd, &flags, nullptr, nullptr);
			if (bytesRecvd > 0 && rc != SOCKET_ERROR && ObjectsInitialized.test(std::memory_order::relaxed)) {
				Lock lock(TriDataMutex);
				for (int i = 0; i < COUNT_TRIANGLES; i++) {
					const auto& [SpatialData, PhysicsData] = TriData[i];
					Triangles[i]->SetTransform(b2Vec2(SpatialData[0], SpatialData[1]), SpatialData[2]);
					Triangles[i]->SetLinearVelocity(b2Vec2(PhysicsData[0], PhysicsData[1]));
					Triangles[i]->SetAngularVelocity(PhysicsData[2]);
				}
			}
		}

		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}
}