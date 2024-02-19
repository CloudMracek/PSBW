# Function to read YAML file and generate CMake code
function(read_yaml_and_set_defines YAML_FILE)
    file(READ ${YAML_FILE} YAML_CONTENT)

    # Remove comments from YAML content
    string(REGEX REPLACE "#.*" "" YAML_CONTENT ${YAML_CONTENT})

    # Split YAML content into lines
    string(REPLACE "\n" ";" YAML_CONTENT ${YAML_CONTENT})

    # Iterate over each line
    foreach(line ${YAML_CONTENT})
        # Split line by colon
        string(REPLACE ":" ";" parts ${line})
        list(GET parts 0 key)
        list(REMOVE_AT parts 0)
        string(JOIN ":" value ${parts})

        # Remove leading and trailing whitespace from key and value
        string(STRIP ${key} key)
        string(STRIP ${value} value)

        # Generate CMake code to set the define
        target_compile_definitions(${PROJECT_NAME} PRIVATE ${key}=${value})
    endforeach()
endfunction()

