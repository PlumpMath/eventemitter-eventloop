find_path (GSL_INCLUDE_DIR gsl.h PATH_SUFFIXES include PATHS
    ${GSL_DIR}
    $ENV{GSL_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
    /devel
)
set (GSL_FOUND "NO")

if (GSL_INCLUDE_DIR)
    set(GSL_FOUND "YES")
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GSL DEFAULT_MSG GSL_INCLUDE_DIR)