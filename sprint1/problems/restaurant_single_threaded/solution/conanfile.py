from conan import ConanFile
from conan.tools.cmake import cmake_layout

class RestaurantConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    requires = "boost/1.78.0"

    def layout(self):
        cmake_layout(self)
