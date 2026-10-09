#pragma once

#include <stddef.h>

#include "union.h"

#ifndef CERIVE_IF_DECL
#	if defined(__clang__)
#		define CERIVE_IF_DECL 0
#	elif defined(__GNUC__) && __GNUC__ >= 15
#		define CERIVE_IF_DECL 1
#	elif defined(__STDC_VERSION__) && __STDC_VERSION__ > 202311L
#		define CERIVE_IF_DECL 1
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
			if ([[maybe_unused]] variant const * const bind = &cerive_matched->variant)
#	define CERIVE_LET(instance, variant, bind) ( \
		[[maybe_unused]] variant const * const bind = ( \
			CERIVE_IS(instance, variant) ? &(instance).variant : (variant const *) NULL \
		) \
	)

#else

#	define CERIVE_P_NEEDS_IF_DECL \
		static_assert( \
			0, \
			"cerive MATCH/CASE/if-let require N3356 if-declarations; define CERIVE_IF_DECL 1 " \
			"if this compiler has them. The rest of cerive is portable C23" \
		)
#	define CERIVE_MATCH(instance) CERIVE_P_NEEDS_IF_DECL
#	define CERIVE_CASE(variant, bind) CERIVE_P_NEEDS_IF_DECL
#	define CERIVE_LET(instance, variant, bind) (cerive_if_let_requires_N3356_if_declarations)

#endif

#ifndef CERIVE_NO_SHORT_NAMES
#	define MATCH CERIVE_MATCH
#	define CASE CERIVE_CASE
#	define LET CERIVE_LET
#endif
