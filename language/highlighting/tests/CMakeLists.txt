set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(highlightingtest_SRCS test_highlighting.cpp)

kde4_add_unit_test(highlightingtest ${highlightingtest_SRCS})
target_link_libraries(highlightingtest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} 
    ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)
