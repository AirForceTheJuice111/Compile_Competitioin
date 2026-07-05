cmake_minimum_required(VERSION 3.20)
project(SysY2022ContestCompiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-w)

option(BUILD_LEGACY_FMJ "Build legacy FMJ compiler tools kept for migration debugging" OFF)

add_executable(compiler tools/compiler/main.cc)

if(NOT BUILD_LEGACY_FMJ)
    return()
endif()

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.8 REQUIRED)

include_directories(include/ast)
include_directories(include/frontend)
include_directories(include/ir)
include_directories(include/quad)
include_directories(include/quadflow)
include_directories(include/opt)
include_directories(include/util)
include_directories(include/instr)
include_directories(include/reg)
include_directories(vendor/tinyxml2)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
FLEX_TARGET(FMJLexer lib/frontend/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/generated/lexer.cc)
BISON_TARGET(FMJParser lib/frontend/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.tab.hh)
ADD_FLEX_BISON_DEPENDENCY(FMJLexer FMJParser)

set(FINAL_SOURCES
    ${FLEX_FMJLexer_OUTPUTS}
    ${BISON_FMJParser_OUTPUTS}
    vendor/tinyxml2/tinyxml2.cc
    lib/ast/FDMJAST.cc
    lib/ast/namemaps.cc
    lib/ast/setnamemaps.cc
    lib/ast/semantanlyzer.cc
    lib/ir/ast2tree.cc
    lib/ir/canon.cc
    lib/ir/tr_exp.cc
    lib/ir/treep.cc
    lib/quad/blocking.cc
    lib/quad/quad.cc
    lib/quad/quadssa.cc
    lib/quad/quadssa_diag.cc
    lib/quad/tree2quad.cc
    lib/quadflow/controlflowinfo.cc
    lib/quadflow/dataflowinfo.cc
    lib/quadflow/flowinfo.cc
    lib/opt/defusechain.cc
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
    lib/util/ast2xml.cc
    lib/util/flow2xml.cc
    lib/util/print_config.cc
    lib/util/quad2xml.cc
    lib/util/source_printer.cc
    lib/util/tree2xml.cc
    lib/util/xml2asmprog.cc
    lib/util/xml2ast.cc
    lib/util/xml2flow.cc
    lib/util/xml2quad.cc
    lib/util/xml2tree.cc
    lib/instr/advDFG.cc
    lib/instr/asmdataflow.cc
    lib/instr/asmprog.cc
    lib/instr/asmprog2colored.cc
    lib/instr/buildAdvDFG.cc
    lib/instr/buildPreSchedule.cc
    lib/instr/fileSchedule.cc
    lib/instr/preSchedule.cc
    lib/instr/predataflowpass.cc
    lib/instr/schedule.cc
    lib/instr/selectInstr.cc
    lib/reg/buildIg.cc
    lib/reg/coloring.cc
    lib/reg/ig.cc
    lib/reg/simcoafrespisel.cc
)

add_executable(fmjcc tools/fmjcc/main.cc ${FINAL_SOURCES})

add_executable(fmjinterp tools/fmjinterp/main.cc ${FINAL_SOURCES})
