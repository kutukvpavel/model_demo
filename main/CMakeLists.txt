idf_component_register(
    SRCS 
    "model_data.cpp"
    "test_data.cpp"
    "main.cpp"
    
    INCLUDE_DIRS 
    "."

    REQUIRES
    model
    macros
    tfmicro
    eigen
    console
    )
