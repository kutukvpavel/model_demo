idf_component_register(
    SRCS "preprocessing.cpp" "my_model.cpp" "temperature_profile.cpp"
    INCLUDE_DIRS "."
    REQUIRES tfmicro eigen macros
    )
