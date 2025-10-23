idf_component_register(
    SRCS 
    "main.cpp"
    "model_data.cpp"
    
    INCLUDE_DIRS 
    "."

    REQUIRES
    model
    macros
    tfmicro
    eigen
    console
    )
