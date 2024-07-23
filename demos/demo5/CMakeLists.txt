include_directories(

)

set(demo_SRCS
    demo5.cpp
)

add_definitions(-DWITH_SMIME)

add_executable(demo5
    ${demo_SRCS}
)

target_link_libraries(demo5
    SimpleMail::Core
    Qt${QT_VERSION_MAJOR}::Core
    OpenSSL::SSL
    OpenSSL::Crypto
)
