
set(libnode_SRCS
node.h
node.cpp
)

# Declare the library
add_library(libnode STATIC
${libnode_SRCS}
)

# Specify here the include directories exported
# by this library
target_include_directories(libnode PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)