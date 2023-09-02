FILE(GLOB_RECURSE main_source ./*.*)
idf_component_register(SRCS ${main_source}
        INCLUDE_DIRS "" "./Functions/")
