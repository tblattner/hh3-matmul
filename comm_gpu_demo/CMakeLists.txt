cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

include(FetchContent)

# Uintah2::Comm
FetchContent_Declare(
        Uintah2
        GIT_REPOSITORY https://github.com/nitishingde/Uintah2
        GIT_TAG		   main
)
FetchContent_MakeAvailable(Uintah2)

# Serialization: cereal
set(SKIP_PERFORMANCE_COMPARISON ON CACHE BOOL "Skip building performance sandbox comparison (requires boost)" ON)
FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal.git
        GIT_TAG		   master
)
FetchContent_MakeAvailable(cereal)

add_library(hh3_matmul
        mm.h
        utility.h

        data/cuda_block_data.h
        data/cuda_matrix_block_data.h
        data/data_type.h
        data/matrix_block_data.h
        data/matrix_data.h
        data/matrix_meta_data.h
        data/serialization.h

        execution_pipeline/inner_product_exec_pipeline.h
        execution_pipeline/multi_gpu_exec_pipeline.h

        graph/gpu_computation_graph.h
        graph/inner_product_cuda_graph.h

        state/cuda_input_block_state.h
        state/inner_product_computation_state.h
        state/output_state.h
        state/partial_computation_state.h
        state/partial_computation_state_manager.h

        task/comm_task.h
        task/contiguous_matrix_block_generator_task.h
        task/cuda_copy_in_gpu_task.h
        task/cuda_copy_out_gpu_task.h
        task/cuda_product_task.h
        task/inner_product_task.h
        task/matrix_column_traversal_task.h
        task/matrix_row_traversal_task.h
        task/pair_generator_task.h
        task/addition_task.h
)

target_compile_definitions(hh3_matmul
    PUBLIC
        HH_USE_CUDA
)

target_include_directories(hh3_matmul
    PUBLIC
        ${CUDA_INCLUDE_DIRS}
        ${Hedgehog_INCLUDE_DIRS}
)

target_link_libraries(hh3_matmul
    PUBLIC
        ${CUDA_CUBLAS_LIBRARIES}
        ${CUDA_LIBRARIES}
        ${Hedgehog_LIBRARIES}
        cereal::cereal
        Uintah2::comm
)

add_executable(outer_prod outer_prod.cpp)
target_link_libraries(outer_prod
    PUBLIC
        hh3_matmul
)

add_executable(inner_prod inner_prod.cpp)
target_link_libraries(inner_prod
    PUBLIC
        hh3_matmul
)

add_executable(comm_outer_prod comm_outer_prod.cpp)
target_link_libraries(comm_outer_prod
    PUBLIC
        hh3_matmul
)

add_executable(mpi_outer_prod mpi_outer_prod.cpp)
target_link_libraries(mpi_outer_prod
    PUBLIC
        hh3_matmul
)

add_executable(compare_prod compare.cpp)
target_link_libraries(compare_prod
    PUBLIC
        hh3_matmul
)
