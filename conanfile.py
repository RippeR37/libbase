from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain


class LibbaseRecipe(ConanFile):
    name = "libbase"
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "module_net": [True, False],
        "module_win": [True, False],
        "module_wx": [True, False],

        "examples": [True, False],
        "tests": [True, False],
        "docs": [True, False],

        "with_tidy": [True, False],
        "with_asan": [True, False],
        "with_tsan": [True, False],
    }
    default_options = {
        "module_net": True,
        "module_win": True,
        "module_wx": False,

        "examples": True,
        "tests": True,
        "docs": False,

        "with_tidy": True,
        "with_asan": False,
        "with_tsan": False,
    }

    def config_options(self):
        if self.settings.os != "Windows":
            del self.options.module_win

    def validate(self):
        if self.settings.os not in ["Windows", "Linux", "Macos"]:
            raise ConanInvalidConfiguration("Unsupported OS")
        if self.options.with_asan and self.options.with_tsan:
            raise ConanInvalidConfiguration("ASAN and TSAN cannot be used together")
        check_min_cppstd(self, "17")

    def requirements(self):
        self.requires("glog/[~0.7]", transitive_headers=True, transitive_libs=True)
        if self.options.module_net:
            self.requires("libcurl/[>=8.12 <9.0]")
        if self.options.module_wx:
            self.requires("wxwidgets/[>=3.2 <4.0]")

        if self.options.tests:
            self.requires("gtest/[~1.16]")
            self.requires("benchmark/[~1.9]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["LIBBASE_BUILD_MODULE_NET"] = self.options.module_net
        if self.settings.os == "Windows":
            tc.cache_variables["LIBBASE_BUILD_MODULE_WIN"] = self.options.module_win
        tc.cache_variables["LIBBASE_BUILD_MODULE_WX"] = self.options.module_wx
        tc.cache_variables["LIBBASE_BUILD_EXAMPLES"] = self.options.examples
        tc.cache_variables["LIBBASE_BUILD_TESTS"] = self.options.tests
        tc.cache_variables["LIBBASE_BUILD_PERFORMANCE_TESTS"] = self.options.tests
        tc.cache_variables["LIBBASE_BUILD_DOCS"] = self.options.docs
        tc.cache_variables["LIBBASE_CLANG_TIDY"] = self.options.with_tidy
        tc.cache_variables["LIBBASE_BUILD_ASAN"] = self.options.with_asan
        tc.cache_variables["LIBBASE_BUILD_TSAN"] = self.options.with_tsan
        tc.generate()
