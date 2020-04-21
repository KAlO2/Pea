#ifndef PEA_IO_BAD_TYPE_EXCEPTION_H_
#define PEA_IO_BAD_TYPE_EXCEPTION_H_

#include <stdexcept>

namespace pea {

class BadTypeException: public std::runtime_error
{
public:
	explicit BadTypeException(const std::string& what);
	
	explicit BadTypeException(const char* what);
	
	virtual ~BadTypeException() noexcept = default;
};

}  // namespace pea
#endif  // PEA_IO_BAD_TYPE_EXCEPTION_H_
