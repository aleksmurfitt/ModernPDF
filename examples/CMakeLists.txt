set(TARGET epdf)
add_executable(${TARGET} test.cpp)
target_link_libraries(${TARGET} PRIVATE pdflib)

# Sign with an entitlements file for debugging on mac

set(MAKE_ENTITLEMENTS /usr/libexec/PlistBuddy -c "Add :com.apple.security.get-task-allow bool true" tmp.entitlements || true)
set(SIGN_BINARY codesign -s - --entitlements tmp.entitlements -f $<TARGET_FILE:epdf>)
set(MAC_DEBUG "$<$<PLATFORM_ID:Darwin>:$<CONFIG:Debug,RelWithDebInfo>>")
add_custom_command(TARGET epdf POST_BUILD
    COMMAND "$<${MAC_DEBUG}:${MAKE_ENTITLEMENTS}>"
    COMMAND "$<${MAC_DEBUG}:${SIGN_BINARY}>"
    COMMAND_EXPAND_LISTS
)
