#include "io/BadTypeException.h"

using namespace pea;

BadTypeException::BadTypeException(const std::string& what):
		runtime_error(what)
{
}

BadTypeException::BadTypeException(const char* what):
		runtime_error(what)
{
}
