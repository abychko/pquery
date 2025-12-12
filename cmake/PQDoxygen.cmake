IF(BUILD_DOCS)

FIND_PACKAGE(Doxygen REQUIRED dot
  OPTIONAL_COMPONENTS mscgen dia)

IF(DOXYGEN_VERSION VERSION_LESS "1.9.1")
    message(WARNING "Doxygen ${DOXYGEN_VERSION} found. Version 1.9.1+ recommended for better C++ support.")
ENDIF()

# Configure Doxygen settings
SET(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/doc")
SET(DOXYGEN_GENERATE_HTML YES)
SET(DOXYGEN_HTML_OUTPUT "html")
SET(DOXYGEN_GENERATE_LATEX NO)

# Project information
set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
set(DOXYGEN_PROJECT_BRIEF "PQuery Project Documentation")
set(DOXYGEN_OUTPUT_LANGUAGE "English")

# Enable diagrams and UML
set(DOXYGEN_HAVE_DOT YES)
set(DOXYGEN_CLASS_DIAGRAMS YES)
set(DOXYGEN_CLASS_GRAPH YES)
set(DOXYGEN_COLLABORATION_GRAPH YES)
# set(DOXYGEN_UML_LOOK YES)
set(DOXYGEN_TEMPLATE_RELATIONS YES)

# Diagram settings
set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
set(DOXYGEN_DOT_TRANSPARENT YES)
set(DOXYGEN_DOT_MULTI_TARGETS YES)
set(DOXYGEN_GRAPHICAL_HIERARCHY YES)
set(DOXYGEN_DIRECTORY_GRAPH YES)
set(DOXYGEN_INCLUDE_GRAPH YES)
set(DOXYGEN_INCLUDED_BY_GRAPH YES)

# Font and styling for diagrams
set(DOXYGEN_DOT_FONTNAME "Arial")
set(DOXYGEN_DOT_FONTSIZE 10)
set(DOXYGEN_DOT_NODE_FONTNAME "Helvetica")
set(DOXYGEN_DOT_NODE_FONTSIZE 11)
set(DOXYGEN_DOT_EDGE_FONTNAME "Helvetica")
set(DOXYGEN_DOT_EDGE_FONTSIZE 9)

# Node styling
set(DOXYGEN_DOT_NODE_SHAPE "box")
set(DOXYGEN_DOT_NODE_STYLE "rounded,filled")
set(DOXYGEN_DOT_NODE_COLOR "lightblue")

# Edge styling
set(DOXYGEN_DOT_EDGE_COLOR "darkgray")
set(DOXYGEN_DOT_EDGE_STYLE "solid")

# Layout direction
set(DOXYGEN_DOT_RANK_DIR "TB")  # Top to Bottom (LR for Left to Right)

# Performance and size limits
set(DOXYGEN_DOT_GRAPH_MAX_NODES 200)
set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 3)
set(DOXYGEN_DIR_GRAPH_MAX_DEPTH 3)

# Extract all information
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
set(DOXYGEN_EXTRACT_ANON_NSPACES YES)
set(DOXYGEN_HIDE_UNDOC_MEMBERS NO)
set(DOXYGEN_HIDE_UNDOC_CLASSES NO)

# C++ specific settings
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_CPP_CLI_SUPPORT NO)
set(DOXYGEN_SIP_SUPPORT NO)
set(DOXYGEN_IDL_PROPERTY_SUPPORT YES)
set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
set(DOXYGEN_GROUP_NESTED_COMPOUNDS YES)
set(DOXYGEN_SUBGROUPING YES)

# Input configuration
set(DOXYGEN_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include")
set(DOXYGEN_RECURSIVE YES)
set(DOXYGEN_FILE_PATTERNS "*.cpp" "*.h" "*.hpp" "*.cc" "*.cxx" "*.ixx")
set(DOXYGEN_EXCLUDE_SYMLINKS YES)

# Exclude patterns
set(DOXYGEN_EXCLUDE_PATTERNS
    "*/test/*"
    "*/tests/*"
    "*/build/*"
    "*/cmake-build-*/*"
    "*/third_party/*"
    "*/external/*"
    "*/vendors/*"
)

# Exclude symbols
set(DOXYGEN_EXCLUDE_SYMBOLS
    "internal_*"
    "*Impl"
    "*Private"
    "*::detail::*"
)

# Search configuration
set(DOXYGEN_SEARCH_INCLUDES YES)
set(DOXYGEN_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(DOXYGEN_STRIP_FROM_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# HTML output customization
set(DOXYGEN_HTML_TIMESTAMP YES)
set(DOXYGEN_HTML_DYNAMIC_SECTIONS YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_FULL_PATH_NAMES NO)
set(DOXYGEN_STRIP_FROM_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Source browser
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_INLINE_SOURCES NO)
set(DOXYGEN_REFERENCED_BY_RELATION YES)
set(DOXYGEN_REFERENCES_RELATION YES)

# Alphabetical index
set(DOXYGEN_ALPHABETICAL_INDEX YES)
set(DOXYGEN_COLS_IN_ALPHA_INDEX 2)

# Documentation quality
set(DOXYGEN_WARNINGS YES)
set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
set(DOXYGEN_WARN_IF_DOC_ERROR YES)
set(DOXYGEN_WARN_NO_PARAMDOC YES)
set(DOXYGEN_WARN_AS_ERROR NO)

# Verbosity
set(DOXYGEN_QUIET NO)

# Define source files for documentation
#set(SOURCES_FOR_DOCS
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
#    # Add more files as needed
#)

# Create documentation target
doxygen_add_docs(
    generate_docs  # Target name
    ${SOURCES_FOR_DOCS}
    ALL            # Include in default build (optional)
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating HTML documentation with class diagrams"
)

# Add install target for documentation
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/doc/html
    DESTINATION share/doc/${PROJECT_NAME}
    COMPONENT documentation
)

# Optional: Create a configuration summary
message(STATUS "")
message(STATUS "=========================================")
message(STATUS "Documentation Configuration Summary")
message(STATUS "=========================================")
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Doxygen: ${DOXYGEN_VERSION}")
message(STATUS "Graphviz: ${DOXYGEN_DOT_FOUND}")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/doc/html/index.html")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  make generate_docs  - Generate full documentation with diagrams")
message(STATUS "=========================================")

ENDIF()
