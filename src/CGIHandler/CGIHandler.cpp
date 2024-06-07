#include "CGIHandler.hpp"

CGIHandler::CGIHandler(std::string script_path)
: script_path_(script_path) {}

Response CGIHandler::execute(Request& request) {
	// TODO
	(void)request;
	return Response();
}
