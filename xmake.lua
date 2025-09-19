--- The /bigobj flag increases the number of sections that an object file 
--- can contain. This is useful when you have very large source files that 
--- result in a large number of sections, which can exceed the default limit.
--- This flag is often necessary when dealing with large templates or
--- heavily templated code in C++.
add_cxxflags("/bigobj")

--- The /FS flag enables file sharing mode, allowing multiple compiler processes
--- to write to the same .PDB file simultaneously.
add_cxxflags("/FS")

add_cxxflags(
	"/W4", --- warning level 4, like -Wall in gcc
	"/analyze",  --- msvc static code analysis support 
	"/permissive-",  --- enforce strict standard conformance
	"/sdl",  --- enable additional security features
	"/external:anglebrackets", --- treat all headers files included in <> as external code
 	"/analyze:external-", --- didn't analyze external code
    '/we4834', -- discard relative warning would turn to error
 {force = true} )
set_policy("build.warning", true)

set_exceptions "cxx"
set_encodings "utf-8"

set_config("pkg_searchdirs", "$(env PROJECTS_PATH)/../.xmake_pkgs")
set_config("vcpkg", "$(env PROJECTS_PATH)/vcpkg")


add_rules("mode.debug", "mode.release")
add_rules("mode.profile", "mode.coverage", "mode.asan", "mode.tsan", "mode.lsan", "mode.ubsan")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "build", lsp = "clangd" })
add_rules("utils.install.pkgconfig_importfiles")
add_rules("utils.install.cmake_importfiles")

add_requireconfs("*", { debug = is_mode("debug") })
add_runenvs("PATH", "$(projectdir)/bin")


add_requires("spdlog", { alias = "spdlog", configs = { header_only = true, debug = is_mode("debug") } }) 
add_requires("fmt", { alias = "fmt", configs = { header_only = true, debug = is_mode("debug") } })
add_requires("conan::boost/1.85.0", { alias = "boost", configs = { debug = is_mode("debug") } })

set_languages "c++20"
add_includedirs "$(buildir)"


add_defines("_UNICODE", "UNICODE", "NOMINMAX", "WIN32_LEAN_AND_MEAN")
add_defines("SPDLOG_FMT_EXTERNAL", "SPDLOG_WCHAR_TO_UTF8_SUPPORT")
add_defines("BOOST_ASIO_HAS_CO_AWAIT", "BOOST_ASIO_HAS_STD_COROUTINE")
add_defines("SPDLOG_FMT_EXTERNAL", "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE", "SPDLOG_WCHAR_TO_UTF8_SUPPORT")

add_cxxflags("cl::/Zc:__cplusplus")
add_cxxflags("/bigobj")

add_packages("spdlog", "fmt", "boost")

target "cc-common"
add_defines("VERSION_MAJOR=1", "VERSION_MINOR=0", "VERSION_ALTER=0")
set_kind("static")
add_files("cc-common/**/*.cpp", "cc-common/*.cpp")
add_includedirs "."
add_links "ole32"

-- target "cc-common-test"
-- add_files("test/*.cpp", "cc-common/**/*.cpp", "cc-common/*.cpp")
-- add_includedirs("test")
-- add_includedirs "."
-- add_defines("UNITTEST") 
-- add_packages("gtest")
-- set_kind("binary")

------------------------ script domain ------------------------
after_build(function(target)
    import("core.project.task")
    task.run("uber_pkg")
end)

task("uber_pkg")
on_run(function()
    import("core.project.project")
    import("core.project.config")
    import("utils.archive")
    import("core.base.option")
    config.load()
    os.cp(project.target("cc-common"):targetdir() .. "/cc-common.lib", "$(projectdir)/bin/cc-common.lib")
end)
