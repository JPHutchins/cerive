#include <cerive/cerive.h>

#define CERIVE_Nop(T)

#if defined(REJECT_THIRTEEN_TRAITS)
#	define Bounded_FIELDS(X) X(int32_t, x)
CERIVE(Bounded, Struct, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop)
#elif defined(REJECT_TWO_ARITY_POINTER)
#	define TwoArity_FIELDS(X) X(int *, p)
CERIVE(TwoArity, Struct)
#else
#	define Bounded_FIELDS(X) X(int32_t, x)
CERIVE(Bounded, Struct, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop)
#	define ThreeArity_FIELDS(X) X(*, int, p)
CERIVE(ThreeArity, Struct)
#endif
