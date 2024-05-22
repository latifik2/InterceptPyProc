#include "Debug.h"
using namespace dbg;

Debug::Debug(std::string optitonalMessage) : optitonalMessage(optitonalMessage) {}
void Debug::GetLastErrorAsString(std::string optitonalMessage) {
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return; //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	if (optitonalMessage.size() > 0)
		std::cout << optitonalMessage << std::endl;
	std::cout << message << std::endl;
}
