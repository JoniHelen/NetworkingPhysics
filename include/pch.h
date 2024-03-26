#pragma once

#define GLFW_INCLUDE_NONE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <box2d/box2d.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <linmath.h>
#include <imgui/imgui.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <WinSock2.h>
#include <WS2tcpip.h>

using AddressInfo = ADDRINFOT;
using Buffer = WSABUF;
using Socket = SOCKET;
using WSAData = WSADATA;
using RunningFlag = std::atomic_flag;
using Lock = std::lock_guard<std::mutex>;