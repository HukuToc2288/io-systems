cmake_minimum_required(VERSION 3.17)
project(lab1io C)

set(CMAKE_C_STANDARD 99)

# Find kernel headers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelHeaders REQUIRED)


# find MODULE_LICENSE("GPL"), MODULE_AUTHOR() etc.
# thanks to "merseyviking" from stack overflow
add_definitions(-D__KERNEL__ -DMODULE)

# this is needed in order for CLion IDE to provide syntax highlightning
# this is independent from the actual kernel object that is built
add_executable(dummy
        # add all *.h and *.c files here that # CLion should cover
        lab1.c
        )

# CLion IDE will find symbols from <linux/*>
target_include_directories("dummy" PRIVATE ${KERNELHEADERS_INCLUDE_DIRS})
