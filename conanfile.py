from conan import ConanFile


class WxpexConan(ConanFile):
    name = "wxpex"
    version = "1.0.0"

    python_requires = "boiler/0.1"
    python_requires_extend = "boiler.LibraryConanFile"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/wxpex"
    description = "A simplified API to combine wxWidgets with pex nodes."
    topics = ("GUI", "C++", "wxWidgets")

    def init(self):
        # Call the base method _only_ if it exists
        base_init = getattr(super(), "init", None)

        if callable(base_init):
            base_init()

        # Extend (not replace) the inherited defaults

        # self.default_options.update({
        #     "wxwidgets/*:stc": False
        # })

        self.default_options["wxwidgets/*:stc"] = False

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.3]")
        self.requires("fields/[~1.4]")
        self.requires("pex/[~1.0]")
        self.requires("tau/[~1.11]")

        # As this library is a wrapper around wxWidgets, it makes sense to me
        # to propagate the headers and libs to make customization easier.
        # Also, it won't break downstream consumers that expected requirements
        # to propagate by default.
        self.requires("wxwidgets/3.2.7.1", transitive_headers=True)
