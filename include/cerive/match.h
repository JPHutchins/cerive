#pragma once

#include <stddef.h>

#include "union.h"

/*
 * Pattern matching for cerive tagged unions -- an opt-in value-add. Import it
 * explicitly (`#include <cerive/match.h>`); <cerive/cerive.h> does NOT pull it in.
 *
 * MATCH/CASE and if-let use N3356 "if declarations" (a declaration as the controlling
 * clause of `if`) -- a C2Y feature (WG14 N3356), NOT C23, that gcc 15+ implements early
 * and clang 21 does not. The declaration binds a `variant const *` scoped to the arm, so
 * a wrong-field access won't compile and `break`/`continue` behave naturally. Everything
 * else in cerive (derive machinery, union construction, IS/NEW) is portable C23 and does
 * not need this header.
 *
 * CERIVE_IF_DECL gates the feature; define it yourself to force on/off. Without N3356,
 * including this header is still safe -- only *using* MATCH/CASE/if-let is a compile error.
 *
 *   #include <cerive/match.h>
 *
 *   MATCH (shape) {
 *       CASE (Point, p) { use(p->x); }
 *       CASE (Frame, f) { use(f->id); }
 *   }
 *
 *   if LET (shape, Point, p) { use(p->x); } else { ... }   // shape is not Point
 */

#ifndef CERIVE_IF_DECL
#	if defined(__STDC_VERSION__) && __STDC_VERSION__ > 202311L
#		define CERIVE_IF_DECL 1 /* N3356 is standard from C2Y */
#	elif defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 15
#		define CERIVE_IF_DECL 1 /* gcc 15+ accepts N3356 as an extension pre-C2Y */
#	else
#		define CERIVE_IF_DECL 0
#	endif
#endif

#if CERIVE_IF_DECL

#	define CERIVE_MATCH(instance) \
		if (typeof(instance) const * cerive_matched = &(instance)) \
			switch (cerive_matched->tag)
#	define CERIVE_CASE(variant, bind) \
		break; \
		case variant##_tag: \
			if (variant const * const bind = &cerive_matched->variant)
#	define CERIVE_LET(instance, variant, bind) ( \
		variant const * const bind = ( \
			CERIVE_IS(instance, variant) ? &(instance).variant : (variant const *) NULL \
		) \
	)

#else

#	define CERIVE_P_NEEDS_IF_DECL \
		static_assert( \
			0, \
			"cerive MATCH/CASE/if-let require N3356 if-declarations (gcc 15+, or -std=c2y); " \
			"the rest of cerive is portable C23" \
		)
#	define CERIVE_MATCH(instance) CERIVE_P_NEEDS_IF_DECL
#	define CERIVE_CASE(variant, bind) CERIVE_P_NEEDS_IF_DECL
#	define CERIVE_LET(instance, variant, bind) (cerive_if_let_requires_N3356_if_declarations)

#endif

/* Short aliases (the one concession to brevity); #define CERIVE_NO_SHORT_NAMES to opt out. */
#ifndef CERIVE_NO_SHORT_NAMES
#	define MATCH CERIVE_MATCH
#	define CASE CERIVE_CASE
#	define LET CERIVE_LET
#endif
