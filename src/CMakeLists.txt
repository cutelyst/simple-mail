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
    server.h
    serverreply.h
    smtpexports.h
    SimpleMail
)

set(simplemailqt_HEADERS_PRIVATE
#     common.h
)

add_library(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    ${simplemailqt_SRC}
    ${simplemailqt_HEADERS}
    ${simplemailqt_HEADERS_PRIVATE}
)

#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(SimpleMail::Core ALIAS SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR})

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(TARGET SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY DEBUG_POSTFIX "d")
endif()

target_compile_definitions(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
  PRIVATE
    PLUGINS_PREFER_DEBUG_POSTFIX=$<CONFIG:Debug>
    QT_NO_KEYWORDS
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_STRICT_ITERATORS
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_QSTRINGBUILDER
    QT_NO_SIGNALS_SLOTS_KEYWORDS
    QT_USE_FAST_OPERATOR_PLUS
    QT_DISABLE_DEPRECATED_BEFORE=0x050f00
)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
      PRIVATE
        SIMPLE_MAIL_QT_STATIC
    )
endif ()

target_include_directories(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/simplemail${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}>
)

set_target_properties(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

target_link_libraries(SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    PUBLIC
        Qt::Core
        Qt::Network
)

set_property(TARGET SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY PUBLIC_HEADER ${simplemailqt_HEADERS})
install(TARGETS SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    EXPORT SimpleMailTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/simplemail${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/SimpleMail COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simplemail-qtX.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}.pc
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SimpleMail${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
