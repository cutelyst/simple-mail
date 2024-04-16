set(demo_SRCS
    demo3.cpp
)

add_executable(demo3
    ${demo_SRCS}
)

target_link_libraries(demo3
    SimpleMail::Core
    Qt::Core
)
