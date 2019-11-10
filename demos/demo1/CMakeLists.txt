set(demo_SRCS
    demo1.cpp
)

add_executable(demo1
    ${demo_SRCS}
)

target_link_libraries(demo1
    SimpleMail2Qt5::Core
    Qt5::Core
)
