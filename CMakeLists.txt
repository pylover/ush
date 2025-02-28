set(sources
  "str.c"
  "cmdring.c"
  "term.c"
  "ush.c"
)


idf_component_register(
  SRCS "${sources}"
  INCLUDE_DIRS "include"
  REQUIRES euart ering
)


target_compile_options(${COMPONENT_LIB} PRIVATE -fms-extensions)

