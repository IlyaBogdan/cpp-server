#include <iostream>

#include "Server/Server.h"

using namespace std;

int main() {

	Server server = Server("80");
	server.run();

	_getwch();
	return 0;
}