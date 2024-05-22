#include "Coordinator.h"

int main() {
	setlocale(LC_ALL, "Russian");
	std::string procName = "python.exe";
	Coordinator coordinator = Coordinator(procName);
	coordinator.MainLoop();
}