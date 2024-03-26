#include <pch.h>
#include <NetworkingPhysics.h>
#include <Netcode.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main(int argc, char* argv[]) {
	if (argc < 2) return 1;

	std::atomic_flag networkRunning {};
	std::atomic_flag timerRunning {};
	std::future<int> networkExitCode;
	std::future<void> timer;

	bool isServer = false;

	if (strcmp(argv[1], "-client") == 0)
		networkExitCode = std::async(NetPhysics::ConnectToServer, std::ref(networkRunning));
	else if (strcmp(argv[1], "-server") == 0)
	{
		isServer = true;
		networkExitCode = std::async(NetPhysics::ListenForClients, std::ref(networkRunning));
		timer = std::async(NetPhysics::TimedSend, 1, std::ref(timerRunning));
	}
	else
		return 1;

	GLFWwindow* window = NetPhysics::InitWindow();
	gladLoadGL(glfwGetProcAddress);
	NetPhysics::InitImGui(window);
	const ImGuiIO& io = ImGui::GetIO(); (void)io;

	const GLuint program = NetPhysics::GenerateShaderProgram("triangle");

	const GLint v_location = glGetUniformLocation(program, "ViewMatrix");
	const GLint p_location = glGetUniformLocation(program, "ProjMatrix");

	GLuint vertexBuffer, transformBuffer, indexBuffer, vertexArray;
	NetPhysics::GenerateTriangleBuffers(program, vertexBuffer, transformBuffer, indexBuffer, vertexArray);

	const auto world = std::make_unique<b2World>(b2Vec2(0, -9.81f));

	NetPhysics::CreateWorldBounds(world);
	NetPhysics::CreatePhysicsTriangles(world);

	NetPhysics::ObjectsInitialized.test_and_set(std::memory_order::acquire);

	const auto rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;

	float gravityModifier = 0;
	float clearColor[3] = { 0.2f, 0.2f, 0.2f };

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Test Window");

		ImGui::SliderFloat("Gravity factor", &gravityModifier, 0, 1);
		ImGui::ColorPicker3("Clear Color", clearColor);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();

		world->SetGravity({ 0, -9.81f * gravityModifier });

		int width, height;
		mat4x4 m, v, p;

		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = static_cast<float>(width) / static_cast<float>(height);

		glViewport(0, 0, width, height);
		glClearColor(clearColor[0], clearColor[1], clearColor[2], 1);
		glClear(GL_COLOR_BUFFER_BIT);

		if (isServer)
			world->Step(1.0f / static_cast<float>(rate), 20, 10);
		else if (NetPhysics::TriDataMutex.try_lock()) {
			world->Step(1.0f / static_cast<float>(rate), 20, 10);
			NetPhysics::TriDataMutex.unlock();
		}

		if (isServer)
			NetPhysics::CollectTriangleData();

		// Set camera
		mat4x4_identity(v);
		mat4x4_translate_in_place(v, 0, 0, 0);
		mat4x4_invert(v, v);

		constexpr float zoom = 7;

		// Projection
		mat4x4_ortho(p, -ratio * zoom, ratio * zoom, -zoom, zoom, 1.0f, -1.0f);

		for (int i = 0; i < NetPhysics::COUNT_TRIANGLES; i++) {
			mat4x4_identity(m);
			auto& pos = NetPhysics::Triangles[i]->GetPosition();
			mat4x4_translate_in_place(m, pos.x, pos.y, 0);
			mat4x4_rotate_Z(m, m, NetPhysics::Triangles[i]->GetAngle());
			mat4x4_dup(NetPhysics::TriangleTransforms[i], m);
		}

		glUseProgram(program);
		glBindVertexArray(vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NetPhysics::COUNT_TRIANGLES * sizeof(mat4x4), NetPhysics::TriangleTransforms);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		glUniformMatrix4fv(v_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(v));
		glUniformMatrix4fv(p_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(p));

		glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr, NetPhysics::COUNT_TRIANGLES);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	networkRunning.test_and_set(std::memory_order::acquire);
	timerRunning.test_and_set(std::memory_order::acquire);
	std::cout << "Networking thread exited with code: " << networkExitCode.get() << "\n";
	if (isServer)
		timer.get();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}