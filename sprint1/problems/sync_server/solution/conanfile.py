from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.cmake import CMakeToolchain

class HelloConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    requires = "boost/1.83.0"

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        tc = CMakeToolchain(self)
        # Принудительно задаём компиляторы
        tc.variables["CMAKE_C_COMPILER"] = "/usr/bin/g++"
        tc.variables["CMAKE_C_COMPILER_WORKS"] = "ON"
        tc.variables["CMAKE_CXX_COMPILER"] = "/usr/bin/g++"
        tc.variables["CMAKE_CXX_COMPILER_WORKS"] = "ON"
        tc.generate()
