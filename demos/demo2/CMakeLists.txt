find_package(Qt5 5.4.0 REQUIRED COMPONENTS
    Widgets
)

set(demo_SRCS
    sendemail.cpp
    demo2.cpp
)

qt5_wrap_ui(demo_SRCS
    sendemail.ui
)

add_executable(demo2
    ${demo_SRCS}
)

target_link_libraries(demo2
    SimpleMail2Qt5::Core
    Qt5::Core
    Qt5::Widgets
)
