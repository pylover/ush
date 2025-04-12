set(sources
  "cmd.c"
  "ansi.c"
  "term.c"
  "ush.c"
)


if (CONFIG_USH_VI)
  list(APPEND sources
    "vi.c"
  )
endif()


idf_component_register(
  SRCS "${sources}"
  INCLUDE_DIRS "include"
  REQUIRES euart ering
)


target_compile_options(${COMPONENT_LIB} PRIVATE -fms-extensions)

