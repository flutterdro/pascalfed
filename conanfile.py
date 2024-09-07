from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/[~11]")
        self.requires("boost/1.86.0")

    def build_requirements(self):
        self.test_requires("catch2/[~3]")
