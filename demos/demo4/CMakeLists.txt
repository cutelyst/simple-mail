set(demo_SRCS
    demo4.cpp
)

add_executable(demo4
    ${demo_SRCS}
)

target_link_libraries(demo4
    SimpleMail2Qt5::Core
    Qt5::Core
)
