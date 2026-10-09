#if defined(REJECT_MATCH_WITHOUT_IF_DECL) || defined(REJECT_LET_WITHOUT_IF_DECL)
#	define CERIVE_IF_DECL 0
#endif

#include <cerive/cerive.h>
#include <cerive/match.h>

#define CERIVE_Nop(T)

#define Unit_FIELDS(X) X(int32_t, x)
CERIVE(Unit, Struct, Debug)
#define Either_VARIANTS(X) X(Unit)
CERIVE_UNION(Either)

#if CERIVE_IF_DECL || defined(REJECT_MATCH_WITHOUT_IF_DECL)
int either_x(Either const e) {
	MATCH(e) {
		CASE(Unit, u) {
			return u->x;
		}
	}
	return 0;
}
#endif

#if defined(REJECT_THIRTEEN_TRAITS)
#	define Bounded_FIELDS(X) X(int32_t, x)
CERIVE(Bounded, Struct, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop)
#elif defined(REJECT_TWO_ARITY_POINTER)
#	define TwoArity_FIELDS(X) X(int *, p)
CERIVE(TwoArity, Struct)
#elif defined(REJECT_NULL_DEBUG_BUFFER)
int unit_debug_into_null(Unit const * const u) {
	return Unit_debug(u, 1, NULL);
}
#elif defined(REJECT_LET_WITHOUT_IF_DECL)
int either_x(Either const e) {
	if LET(e, Unit, u) {
		return u->x;
	}
	return 0;
}
#elif defined(REJECT_MULTIWORD_SCALAR)
#	define Wide_FIELDS(X) X(long long, v)
CERIVE(Wide, Struct)
#else
#	define Bounded_FIELDS(X) X(int32_t, x)
CERIVE(Bounded, Struct, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop, Nop)
#	define ThreeArity_FIELDS(X) X(*, int, p)
CERIVE(ThreeArity, Struct)
typedef long long long_long;
#	define Wide_FIELDS(X) X(long_long, v)
CERIVE(Wide, Struct, Debug)
#endif
