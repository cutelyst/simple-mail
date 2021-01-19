set(demo_SRCS
    demo1.cpp
)

add_executable(demo1
    ${demo_SRCS}
)

target_link_libraries(demo1
    SimpleMail2Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Core
)
