cmake_minimum_required(VERSION 3.8.2)
project(minishell)

add_executable(
        minishell
        msh.c
        parser.y
        scanner.l
        y.c)

add_custom_target(main COMMAND make -C ${minishell_SOURCE_DIR} CLION_EXE_DIR=${PROJECT_BINARY_DIR})