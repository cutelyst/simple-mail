set(simplemailqt_SRC
    emailaddress.cpp
    emailaddress_p.h
    mimeattachment.cpp
    mimecontentformatter.cpp
    mimefile.cpp
    mimehtml.cpp
    mimeinlinefile.cpp
    mimemessage.cpp
    mimemessage_p.h
    mimemultipart.cpp
    mimemultipart_p.h
    mimepart.cpp
    mimepart_p.h
    mimetext.cpp
    quotedprintable.cpp
    sender.cpp
    sender_p.h
    server.cpp
    server_p.h
    serverreply.cpp
    serverreply_p.h
    smtpexports.h
)

set(simplemailqt_HEADERS
    emailaddress.h
    mimeattachment.h
    mimecontentformatter.h
    mimefile.h
    mimehtml.h
    mimeinlinefile.h
    mimemessage.h
    mimemultipart.h
    mimepart.h
    mimetext.h
    quotedprintable.h
    sender.h
    server.h
    serverreply.h
    smtpexports.h
    SimpleMail
)

set(simplemailqt_HEADERS_PRIVATE
#     common.h
)

add_library(SimpleMail2Qt5 SHARED
    ${simplemailqt_SRC}
    ${simplemailqt_HEADERS}
    ${simplemailqt_HEADERS_PRIVATE}
)

#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(SimpleMail${PROJECT_VERSION_MAJOR}Qt5::Core ALIAS SimpleMail2Qt5)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(TARGET SimpleMail2Qt5 PROPERTY DEBUG_POSTFIX "d")
endif()

target_compile_definitions(SimpleMail2Qt5
  PRIVATE
    PLUGINS_PREFER_DEBUG_POSTFIX=$<CONFIG:Debug>
)

target_include_directories(SimpleMail2Qt5 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/simplemail2-qt5>
)

set_target_properties(SimpleMail2Qt5 PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

target_link_libraries(SimpleMail2Qt5
    PUBLIC
        Qt5::Core
        Qt5::Network
)

set_property(TARGET SimpleMail2Qt5 PROPERTY PUBLIC_HEADER ${simplemailqt_HEADERS})
install(TARGETS SimpleMail2Qt5
    EXPORT SimpleMailTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/simplemail2-qt5/SimpleMail COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simplemail-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt5.pc
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt5.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
