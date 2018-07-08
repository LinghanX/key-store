file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testnode ${SRCS})

TARGET_LINK_LIBRARIES(testnode
    libnode
    libgtest
    libgmock
)

add_test(NAME testnode
         COMMAND testnode)
