from conan import ConanFile

class HelloConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    requires = "boost/1.83.0"

    def generate(self):
        pass
