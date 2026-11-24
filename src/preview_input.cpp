#include <winsock2.h>

#include "preview_input.h"

#include "dashboard_ui.h"

#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <utility>

namespace {
	constexpr unsigned short PREVIEW_PORT = 8765;

	void receive_values(SOCKET socket, SharedData& shared_data){
		char buffer[1024];
		const int size = recvfrom(socket, buffer, sizeof(buffer), 0, nullptr, nullptr);
		if(size <= 0){
			return;
		}

		std::istringstream input(std::string(buffer, size));
		input.imbue(std::locale::classic());
		std::vector<std::shared_ptr<DataField>> points;
		while(input >> std::ws && !input.eof()){
			std::string title;
			double value;
			if(!(input >> std::quoted(title) >> value) || title.empty() ||
				!std::isfinite(value) || std::abs(value) > INT_MAX){
				return;
			}
			auto field = std::make_shared<DataField>(std::move(title), 1, 0, 0, 0, 0, 0, "");
			field->value = value;
			points.push_back(std::move(field));
		}
		if(!points.empty()){
			shared_data.update_data(std::move(points));
		}
	}
}

int run_preview_input(std::shared_ptr<SharedData> shared_data){
	WSADATA winsock;
	const int startup_error = WSAStartup(MAKEWORD(2, 2), &winsock);
	if(startup_error != 0){
		std::cerr << "Could not initialize preview input (Winsock error " << startup_error << ").\n";
		return 1;
	}

	const SOCKET receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = htons(PREVIEW_PORT);
	u_long nonblocking = 1;
	if(receiver == INVALID_SOCKET || ioctlsocket(receiver, FIONBIO, &nonblocking) == SOCKET_ERROR ||
		bind(receiver, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR){
		std::cerr << "Could not open preview input on 127.0.0.1:" << PREVIEW_PORT
			<< " (Winsock error " << WSAGetLastError() << ").\n";
		if(receiver != INVALID_SOCKET){
			closesocket(receiver);
		}
		WSACleanup();
		return 1;
	}

	std::cout << "Preview input listening on 127.0.0.1:" << PREVIEW_PORT << std::endl;
	const int result = run_dashboard_ui(std::move(shared_data), [receiver](SharedData& data){
		receive_values(receiver, data);
	});
	closesocket(receiver);
	WSACleanup();
	return result;
}
