cmake_minimum_required(VERSION 3.20)
project(SysY2022ContestCompiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-w)

set(BACKEND_SOURCES
    lib/backend/backend_driver.cc
    lib/ir/canon.cc
    lib/ir/treep.cc
    lib/quad/blocking.cc
    lib/quad/quad.cc
    lib/quad/quadssa.cc
    lib/quad/quadssa_diag.cc
    lib/quad/tree2quad.cc
    lib/quadflow/controlflowinfo.cc
    lib/quadflow/dataflowinfo.cc
    lib/quadflow/flowinfo.cc
    lib/opt/algebrasimp.cc
    lib/opt/copyprop.cc
    lib/opt/defusechain.cc
    lib/opt/funcspec.cc
    lib/opt/gvn.cc
    lib/opt/inline.cc
    lib/opt/loopheaderwithflow.cc
    lib/opt/loophoistfunc.cc
    lib/opt/loopinductionbasic.cc
    lib/opt/loopinductionderived.cc
    lib/opt/loopinductionelimination.cc
    lib/opt/loopinductionopt.cc
    lib/opt/loopoptanalysis.cc
    lib/opt/loopoptreport.cc
    lib/opt/loopstrengthreduction.cc
    lib/opt/opt.cc
)

add_executable(compiler
    tools/compiler/main.cc
    lib/sysy/lexer.cc
    lib/sysy/lower_tree.cc
    lib/sysy/parallel_plan.cc
    lib/sysy/parser.cc
    lib/sysy/semantics.cc
    vendor/tinyxml2/tinyxml2.cc
    ${BACKEND_SOURCES}
)
target_include_directories(compiler PRIVATE
    include/backend
    include/sysy
    include/ir
    include/quad
    include/quadflow
    include/opt
    vendor/tinyxml2
)
