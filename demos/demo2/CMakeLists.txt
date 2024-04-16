find_package(Qt${QT_VERSION_MAJOR} 5.5.0 COMPONENTS Widgets REQUIRED)

set(demo_SRCS
    sendemail.cpp
    demo2.cpp
)

if (${QT_VERSION_MAJOR} EQUAL 5)
    qt5_wrap_ui(demo_SRCS
        sendemail.ui
        )
else ()
    qt6_wrap_ui(demo_SRCS
        sendemail.ui
    )
endif ()

add_executable(demo2
    ${demo_SRCS}
)

target_link_libraries(demo2
    SimpleMail::Core
    Qt::Core
    Qt::Widgets
)
