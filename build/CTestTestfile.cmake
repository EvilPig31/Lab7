# CMake generated Testfile for 
# Source directory: /workspaces/Lab7
# Build directory: /workspaces/Lab7/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(MyProjectTests "/workspaces/Lab7/build/tests")
set_tests_properties(MyProjectTests PROPERTIES  _BACKTRACE_TRIPLES "/workspaces/Lab7/CMakeLists.txt;42;add_test;/workspaces/Lab7/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
