#ifndef PEA_VIEW_VISIBILITY_H_
#define PEA_VIEW_VISIBILITY_H_

namespace pea {

enum class Visibility: unsigned char
{
	VISIBLE   = 0x00000000,
	INVISIBLE = 0x00000004,
	GONE      = 0x00000008,
};

}  // namespace pea
#endif  // PEA_VIEW_VISIBILITY_H_
