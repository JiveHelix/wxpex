
from conans import ConanFile, CMake, tools


class WxpexConan(ConanFile):
    name = "wxpex"
    version = "0.7.5"

    scm = {
        "type": "git",
        "url": "https://github.com/JiveHelix/wxpex.git",
        "revision": "auto",
        "submodule": "recursive"}

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/wxpex"
    description = "A simplified API to combine wxWidgets with pex nodes."
    topics = ("GUI", "C++", "wxWidgets")

    settings = "os", "compiler", "build_type", "arch"

    generators = "cmake"

    no_copy_source = True

    default_options = {
        "wxwidgets:exceptions": False}

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includes = ["include"]
        self.cpp_info.libs = ["wxpex"]

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.0]")
        self.requires("fields/[~1]")
        self.requires("tau/[~1.7]")
        self.requires("pex/[~0.8]")
        self.requires("wxwidgets/3.2.1@jivehelix/stable")
