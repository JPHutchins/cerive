# A build step, not configure-time, so configures stay side-effect-free.

find_program(UV uv REQUIRED)

set(PYDIR ${CMAKE_SOURCE_DIR}/cmake/python)
set(CSTRUCTS_VENV ${CMAKE_BINARY_DIR}/cstructs-venv)
set(CSTRUCTS ${CSTRUCTS_VENV}/bin/cstructs)
set(CSTRUCTS_STAMP ${CSTRUCTS_VENV}/cstructs.stamp)

file(GLOB CSTRUCTS_SRCS CONFIGURE_DEPENDS ${PYDIR}/src/cstructs/*.py)

add_custom_command(
	OUTPUT ${CSTRUCTS_STAMP}
	COMMAND ${CMAKE_COMMAND} -E env UV_PROJECT_ENVIRONMENT=${CSTRUCTS_VENV}
		${UV} sync --frozen --no-dev --project ${PYDIR}
	COMMAND ${CMAKE_COMMAND} -E touch ${CSTRUCTS_STAMP}
	DEPENDS ${PYDIR}/uv.lock ${PYDIR}/pyproject.toml
	COMMENT "cstructs: uv sync --frozen (uv.lock changed)"
	VERBATIM)
