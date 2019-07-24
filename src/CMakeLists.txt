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
    mimetext_p.h
    quotedprintable.cpp
    sender.cpp
    sender_p.h
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
    smtpexports.h
    SmtpMime
)

set(simplemailqt_HEADERS_PRIVATE
#     common.h
)

add_library(SimpleMailQt5 SHARED
    ${simplemailqt_SRC}
    ${simplemailqt_HEADERS}
    ${simplemailqt_HEADERS_PRIVATE}
)

#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(SimpleMailQt5::Core ALIAS SimpleMailQt5)

#Set target properties
target_include_directories(SimpleMailQt5
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/simplemail-qt5>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(SimpleMailQt5 PRIVATE cxx_auto_type)
target_compile_options(SimpleMailQt5 PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

target_link_libraries(SimpleMailQt5
    PUBLIC
        Qt5::Core
        Qt5::Network
)

##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/SimpleMailQt5)

#This is required so that the exported target has the name UpnpQt and not upnpqt
set_target_properties(SimpleMailQt5 PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simplemail-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/simplemail-qt5.pc
  @ONLY
)

install(TARGETS SimpleMailQt5 EXPORT SimpleMailQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simplemail-qt5.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${simplemailqt_HEADERS}
        DESTINATION include/simplemail-qt5/SimpleMail
)
