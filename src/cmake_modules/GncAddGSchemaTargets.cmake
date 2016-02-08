MACRO(ADD_GSCHEMA_TARGETS _TARGET _gschema_INPUTS)
  SET(_gschema_BUILDS "")
  SET(_gschema_VALIDS "")
  SET(CMAKE_COMMAND_TMP "")
  IF (${CMAKE_VERSION} VERSION_GREATER 3.1)
    SET(CMAKE_COMMAND_TMP ${CMAKE_COMMAND} -E env)
  ENDIF()
  FOREACH(file ${_gschema_INPUTS})
    STRING(REPLACE ".xml" ".valid" file_no_xml ${file})
    #SET(_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${file})
    SET(_BUILD_FILE ${DATADIR_BUILD}/glib-2.0/schemas/${file})
    SET(_VALID_FILE ${CMAKE_CURRENT_BINARY_DIR}/${file_no_xml})
    GNC_CONFIGURE2(${file} ${_BUILD_FILE})
    LIST(APPEND _gschema_BUILDS ${_BUILD_FILE})
    LIST(APPEND _gschema_VALIDS ${_VALID_FILE})
    ADD_CUSTOM_COMMAND(
        OUTPUT ${_VALID_FILE}
        COMMAND ${CMAKE_COMMAND_TMP}
          ${GLIB_COMPILE_SCHEMAS} --strict --dry-run --schema-file=${_BUILD_FILE}
        COMMAND ${CMAKE_COMMAND} -E touch ${_VALID_FILE}
        DEPENDS ${_BUILD_FILE}
    )
  ENDFOREACH(file)

  ADD_CUSTOM_TARGET(${_TARGET} DEPENDS ${_gschema_BUILDS} ${_gschema_VALIDS})

  INSTALL(FILES ${_gschema_OUTPUTS} DESTINATION share/glib-2.0/schemas)

ENDMACRO()
