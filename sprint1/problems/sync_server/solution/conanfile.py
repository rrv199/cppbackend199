from conan import ConanFile
from conan.tools.cmake import cmake_layout

class HelloConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    requires = "boost/1.83.0"

    def layout(self):
        cmake_layout(self)
