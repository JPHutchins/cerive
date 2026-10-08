#if defined(REJECT_MATCH_WITHOUT_IF_DECL)
#	define CERIVE_IF_DECL 0
#endif

#include <cerive/cerive.h>
#include <cerive/match.h>

#define CERIVE_Nop(T)

#define Unit_FIELDS(X) X(int32_t, x)
CERIVE(Unit, Struct)
#define Either_VARIANTS(X) X(Unit)
CERIVE_UNION(Either)

int either_x(Either const e) {
	MATCH(e) {
		CASE(Unit, u) {
			return u->x;
		}
	}
	return 0;
}

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
