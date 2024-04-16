set(async_SRCS
    async1.cpp
)

add_executable(async1
    ${async_SRCS}
)

target_link_libraries(async1
    SimpleMail::Core
    Qt::Core
)
