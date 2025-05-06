idf_component_register(
    COMPONENT_NAME
        "esp-corecomponents"

    SRCS
        "src/Exception.cc"
        "src/exceptions.cc"
        "src/Format.cc"
        "src/IoStream.cc"
        "src/NullStream.cc"
        "src/Stream.cc"
        "src/String.cc"
        "src/str.cc"
        "src/SystemError.cc"
        "src/Utf8Sink.cc"

    INCLUDE_DIRS
        "include"
)
