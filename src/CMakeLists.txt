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

add_library(simplemail-qt5 SHARED
    ${simplemailqt_SRC}
    ${simplemailqt_HEADERS}
    ${simplemailqt_HEADERS_PRIVATE}
)
add_library(SimpleMailQt5::Core ALIAS simplemail-qt5)
set_property(TARGET simplemail-qt5 PROPERTY EXPORT_NAME Core)

set_target_properties(simplemail-qt5 PROPERTIES
    VERSION ${SIMPLEMAIL_VERSION}
    SOVERSION ${SIMPLEMAIL_API_LEVEL}
)
target_link_libraries(simplemail-qt5
    Qt5::Core
    Qt5::Network
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simplemail-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/simplemail-qt5.pc
  @ONLY
)

install(TARGETS simplemail-qt5 EXPORT SimpleMailQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simplemail-qt5.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${simplemailqt_HEADERS}
        DESTINATION include/simplemail-qt5/SimpleMail
)
