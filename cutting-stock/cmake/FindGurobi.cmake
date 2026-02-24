# FindGurobi.cmake
# Locates a Gurobi installation and defines:
#   GUROBI_FOUND, GUROBI_INCLUDE_DIRS, GUROBI_LIBRARIES, GUROBI_CXX_LIBRARY

# Search order: GUROBI_HOME env/cache, then /opt/gurobi*
set(_gurobi_search_paths "")
if(DEFINED ENV{GUROBI_HOME})
    list(APPEND _gurobi_search_paths "$ENV{GUROBI_HOME}")
endif()
if(GUROBI_HOME)
    list(APPEND _gurobi_search_paths "${GUROBI_HOME}")
endif()
file(GLOB _gurobi_opt_dirs "/opt/gurobi*/linux64")
list(SORT _gurobi_opt_dirs ORDER DESCENDING)  # prefer newest
list(APPEND _gurobi_search_paths ${_gurobi_opt_dirs})

# Find header
find_path(GUROBI_INCLUDE_DIR
    NAMES gurobi_c++.h
    PATHS ${_gurobi_search_paths}
    PATH_SUFFIXES include
)

# Find C++ library (static)
find_library(GUROBI_CXX_LIBRARY
    NAMES gurobi_c++
    PATHS ${_gurobi_search_paths}
    PATH_SUFFIXES lib
)

# Find main C library (libgurobiXY.so)
file(GLOB _gurobi_c_libs "")
foreach(_dir ${_gurobi_search_paths})
    file(GLOB _libs "${_dir}/lib/libgurobi[0-9]*.so")
    list(APPEND _gurobi_c_libs ${_libs})
endforeach()
if(_gurobi_c_libs)
    list(GET _gurobi_c_libs 0 GUROBI_C_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gurobi
    REQUIRED_VARS GUROBI_INCLUDE_DIR GUROBI_CXX_LIBRARY GUROBI_C_LIBRARY
)

if(GUROBI_FOUND)
    set(GUROBI_INCLUDE_DIRS ${GUROBI_INCLUDE_DIR})
    set(GUROBI_LIBRARIES ${GUROBI_CXX_LIBRARY} ${GUROBI_C_LIBRARY})

    if(NOT TARGET Gurobi::Gurobi)
        add_library(Gurobi::Gurobi INTERFACE IMPORTED)
        set_target_properties(Gurobi::Gurobi PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GUROBI_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${GUROBI_LIBRARIES}"
        )
    endif()
endif()
