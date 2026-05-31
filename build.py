#!/usr/bin/env python3
import argparse
import os
import platform
import shutil
import subprocess
import sys
import zipfile

APP_VERSION = "0.1"


def main():
    parser = argparse.ArgumentParser(description="Build, test, and run the MiniEditor.")
    parser.add_argument(
        "--config",
        default="Debug",
        choices=["Debug", "Release"],
        help="Build configuration (default: Debug)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean build directory before building",
    )
    parser.add_argument(
        "--build-only",
        action="store_true",
        help="Only build, do not run tests or executable",
    )
    parser.add_argument(
        "--no-tests",
        action="store_true",
        help="Disable building and running tests (default for Release)",
    )
    parser.add_argument(
        "--stress-test", action="store_true", help="Build and run stress tests"
    )
    parser.add_argument(
        "--static", action="store_true", help="Link libraries statically"
    )
    parser.add_argument(
        "--package",
        action="store_true",
        help="Create a platform-appropriate package after building",
    )
    parser.add_argument(
        "--package-output-dir",
        default=os.path.join("build", "packages"),
        help="Directory for packaged artifacts (default: build/packages)",
    )
    parser.set_defaults(palloc_treap_nodes=True)
    parser.add_argument(
        "--palloc-treap-nodes",
        dest="palloc_treap_nodes",
        action="store_true",
        help="Allocate implicit_treap nodes from Palloc slab (default: on)",
    )
    parser.add_argument(
        "--no-palloc-treap-nodes",
        dest="palloc_treap_nodes",
        action="store_false",
        help="Disable Palloc-backed implicit_treap node allocation",
    )
    parser.set_defaults(palloc_single_threaded=True)
    parser.add_argument(
        "--palloc-single-threaded",
        dest="palloc_single_threaded",
        action="store_true",
        help="Build Palloc with mutexes disabled (default: on)",
    )
    parser.add_argument(
        "--no-palloc-single-threaded",
        dest="palloc_single_threaded",
        action="store_false",
        help="Build Palloc with normal mutexes enabled",
    )

    args = parser.parse_args()

    # --- Path Setup ---
    project_root = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(project_root, "build", args.config)

    # Executable name handling for Windows
    executable_name = "minieditor"
    if platform.system() == "Windows":
        executable_name += ".exe"
    executable_path = os.path.join(build_dir, executable_name)

    # --- Clean Step ---
    if args.clean:
        temp_build_dir = os.path.join(project_root, "build")
        print(f"=== Cleaning {temp_build_dir} ===")
        if os.path.exists(temp_build_dir):
            shutil.rmtree(temp_build_dir)
        if os.path.isfile(os.path.join(project_root, "compile_commands.json")):
            os.remove(os.path.join(project_root, "compile_commands.json"))
        sys.exit(0)

    # --- Configuration Step ---
    print(f"=== Configuring ({args.config}) ===")
    os.makedirs(build_dir, exist_ok=True)

    # Determine if tests should be enabled
    # Default: Enable tests only in Debug mode
    build_tests = (args.config == "Debug") and not args.no_tests

    cmake_args = [
        f"-DCMAKE_BUILD_TYPE={args.config}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        f"-DMINIEDITOR_BUILD_TESTS={'ON' if build_tests else 'OFF'}",
        f"-DMINIEDITOR_BUILD_STRESS_TESTS={'ON' if args.stress_test else 'OFF'}",
        f"-DMINIEDITOR_STATIC_LINKING={'ON' if args.static else 'OFF'}",
        f"-DMINIEDITOR_USE_PALLOC_FOR_TREAP_NODES={'ON' if args.palloc_treap_nodes else 'OFF'}",
        f"-DMINIEDITOR_PALLOC_SINGLE_THREADED={'ON' if args.palloc_single_threaded else 'OFF'}",
    ]

    # Generator selection: Prefer Ninja if available, else let CMake decide
    cmd_config = ["cmake", "-S", project_root, "-B", build_dir]
    if shutil.which("ninja"):
        cmd_config.extend(["-G", "Ninja"])

    cmd_config.extend(cmake_args)
    subprocess.check_call(cmd_config)

    # --- Build Step ---
    print(f"=== Building ({args.config}) ===")
    cpu_count = os.cpu_count() or 1
    subprocess.check_call(["cmake", "--build", build_dir, "--parallel", str(cpu_count)])

    # --- Post-Build ---
    # Symlink compile_commands.json to root for clangd support
    compile_commands_src = os.path.join(build_dir, "compile_commands.json")
    compile_commands_dst = os.path.join(project_root, "compile_commands.json")

    if os.path.exists(compile_commands_src):
        try:
            if os.path.exists(compile_commands_dst) or os.path.islink(
                compile_commands_dst
            ):
                os.remove(compile_commands_dst)

            try:
                os.symlink(compile_commands_src, compile_commands_dst)
            except OSError:
                # Windows fallback: copy if symlink fails (requires admin usually)
                shutil.copy(compile_commands_src, compile_commands_dst)
        except Exception as e:
            print(f"Warning: Could not link compile_commands.json: {e}")

    if args.build_only:
        print("Build complete.")
        return

    if args.package:
        package_project(project_root, build_dir, args.package_output_dir, args.config)
        print("Package complete.")
        return

    # --- Test Step ---
    if build_tests:
        print("\n=== Running Tests ===")
        try:
            # Create a copy of the environment and add color forcing variables
            env = os.environ.copy()
            env["CTEST_COLOR_OUTPUT"] = "ON"
            env["CLICOLOR_FORCE"] = "1"

            # ctest handles running the registered tests
            subprocess.check_call(
                ["ctest", "--output-on-failure", "--test-dir", build_dir], env=env
            )
        except subprocess.CalledProcessError:
            print("Tests failed.")
            # We don't exit here to allow running the app if desired,
            # mirroring the '|| true' behavior of the original script.

    # --- Stress Test Step ---
    if args.stress_test:
        print("\n=== Running Stress Tests ===")
        stress_src_dir = os.path.join(project_root, "stress_tests")

        if os.path.isdir(stress_src_dir):
            found_tests = False
            for filename in sorted(os.listdir(stress_src_dir)):
                if filename.endswith(".cpp"):
                    found_tests = True
                    test_name = os.path.splitext(filename)[0]
                    stress_exe = os.path.join(build_dir, test_name)

                    if platform.system() == "Windows":
                        stress_exe += ".exe"

                    if os.path.exists(stress_exe):
                        print(f"--- Running {test_name} ---")
                        try:
                            subprocess.check_call([stress_exe])
                        except subprocess.CalledProcessError:
                            print(f"!!! FAILED: {test_name}")
                    else:
                        print(
                            f"Warning: Executable for {test_name} not found (build might have failed)."
                        )

            if not found_tests:
                print("No .cpp files found in stress_tests/.")
        else:
            print("Directory stress_tests/ does not exist.")

    # --- Run Step ---
    print("\n=== Running Application ===")
    if os.path.exists(executable_path):
        try:
            subprocess.check_call([executable_path])
        except subprocess.CalledProcessError as e:
            sys.exit(e.returncode)
    else:
        print(f"Error: Executable not found at {executable_path}")
        sys.exit(1)


def run_checked(command, cwd=None):
    subprocess.check_call(command, cwd=cwd)


def package_project(project_root, build_dir, output_dir, config):
    os.makedirs(output_dir, exist_ok=True)
    system = platform.system()
    if system == "Linux":
        package_appimage(project_root, build_dir, output_dir, config)
        return

    if system == "Windows":
        package_windows_zip(project_root, build_dir, output_dir, config)
        return

    if system == "Darwin":
        package_macos_dmg(project_root, build_dir, output_dir, config)
        return

    raise SystemExit(f"Packaging is not configured for {system}.")


def executable_name_for_system(system):
    return "minieditor.exe" if system == "Windows" else "minieditor"


def package_windows_zip(project_root, build_dir, output_dir, config):
    package_name = f"minieditor-{config}-windows.zip"
    package_path = os.path.join(output_dir, package_name)
    staging_dir = os.path.join(build_dir, "package-staging")
    shutil.rmtree(staging_dir, ignore_errors=True)
    os.makedirs(staging_dir, exist_ok=True)

    exe_name = executable_name_for_system(platform.system())
    source_exe = os.path.join(build_dir, exe_name)
    if not os.path.exists(source_exe):
        raise SystemExit(f"Executable not found at {source_exe}")

    shutil.copy2(source_exe, os.path.join(staging_dir, exe_name))
    shutil.copy2(os.path.join(project_root, "README.md"), os.path.join(staging_dir, "README.md"))
    shutil.copy2(
        os.path.join(project_root, "docs", "BENCHMARKS.md"),
        os.path.join(staging_dir, "BENCHMARKS.md"),
    )

    with zipfile.ZipFile(package_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for root, _, files in os.walk(staging_dir):
            for filename in files:
                abs_path = os.path.join(root, filename)
                rel_path = os.path.relpath(abs_path, staging_dir)
                archive.write(abs_path, rel_path)
    print(f"Wrote {package_path}")


def package_appimage(project_root, build_dir, output_dir, config):
    appdir = os.path.join(build_dir, "AppDir")
    shutil.rmtree(appdir, ignore_errors=True)
    os.makedirs(os.path.join(appdir, "usr", "bin"), exist_ok=True)
    os.makedirs(os.path.join(appdir, "usr", "share", "doc", "minieditor"), exist_ok=True)

    app_name = "minieditor"
    desktop_src = os.path.join(project_root, "packaging", "minieditor.desktop")
    icon_src = os.path.join(project_root, "packaging", "minieditor.svg")
    exe_name = executable_name_for_system(platform.system())
    source_exe = os.path.join(build_dir, exe_name)
    if not os.path.exists(source_exe):
        raise SystemExit(f"Executable not found at {source_exe}")

    shutil.copy2(source_exe, os.path.join(appdir, "usr", "bin", "minieditor"))
    shutil.copy2(
        os.path.join(project_root, "README.md"),
        os.path.join(appdir, "usr", "share", "doc", "minieditor", "README.md"),
    )
    shutil.copy2(
        os.path.join(project_root, "docs", "BENCHMARKS.md"),
        os.path.join(appdir, "usr", "share", "doc", "minieditor", "BENCHMARKS.md"),
    )
    shutil.copy2(desktop_src, os.path.join(appdir, f"{app_name}.desktop"))
    shutil.copy2(icon_src, os.path.join(appdir, f"{app_name}.svg"))

    app_run_path = os.path.join(appdir, "AppRun")
    with open(app_run_path, "w", encoding="utf-8") as handle:
        handle.write("#!/bin/sh\n")
        handle.write('HERE="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"\n')
        handle.write('exec "$HERE/usr/bin/minieditor" "$@"\n')
    os.chmod(app_run_path, 0o755)

    binary_path = os.path.join(appdir, "usr", "bin", "minieditor")
    if os.path.exists(binary_path):
        os.chmod(binary_path, 0o755)

    appimagetool = shutil.which("appimagetool")
    if not appimagetool:
        raise SystemExit(
            "appimagetool is required to create an AppImage. Install it and rerun --package."
        )

    arch = platform.machine().lower()
    if arch in ("x86_64", "amd64"):
        arch = "x86_64"
    elif arch in ("aarch64", "arm64"):
        arch = "aarch64"

    package_name = f"minieditor-{config}-{arch}.AppImage"
    package_path = os.path.join(output_dir, package_name)
    run_checked([appimagetool, appdir, package_path])
    print(f"Wrote {package_path}")


def package_macos_dmg(project_root, build_dir, output_dir, config):
    app_name = "MiniEditor.app"
    app_bundle = os.path.join(build_dir, app_name)
    contents_dir = os.path.join(app_bundle, "Contents")
    macos_dir = os.path.join(contents_dir, "MacOS")
    resources_dir = os.path.join(contents_dir, "Resources")

    shutil.rmtree(app_bundle, ignore_errors=True)
    os.makedirs(macos_dir, exist_ok=True)
    os.makedirs(resources_dir, exist_ok=True)

    source_exe = os.path.join(build_dir, executable_name_for_system("Darwin"))
    if not os.path.exists(source_exe):
        raise SystemExit(f"Executable not found at {source_exe}")

    shutil.copy2(source_exe, os.path.join(macos_dir, "minieditor"))
    os.chmod(os.path.join(macos_dir, "minieditor"), 0o755)
    shutil.copy2(os.path.join(project_root, "README.md"), os.path.join(resources_dir, "README.md"))
    shutil.copy2(
        os.path.join(project_root, "docs", "BENCHMARKS.md"),
        os.path.join(resources_dir, "BENCHMARKS.md"),
    )

    info_plist_path = os.path.join(contents_dir, "Info.plist")
    with open(info_plist_path, "w", encoding="utf-8") as handle:
        handle.write(
            """<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>CFBundleDisplayName</key>
    <string>MiniEditor</string>
    <key>CFBundleExecutable</key>
    <string>minieditor</string>
    <key>CFBundleIdentifier</key>
    <string>com.github.aaslam12.minieditor</string>
    <key>CFBundleName</key>
    <string>MiniEditor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>{version}</string>
    <key>CFBundleVersion</key>
    <string>{version}</string>
</dict>
</plist>
""".format(version=APP_VERSION)
        )

    package_path = os.path.join(output_dir, f"minieditor-{config}-macos.dmg")
    hdiutil = shutil.which("hdiutil")
    if not hdiutil:
        raise SystemExit("hdiutil is required to create a DMG on macOS.")

    run_checked([hdiutil, "create", "-volname", "MiniEditor", "-srcfolder", app_bundle, "-ov", "-format", "UDZO", package_path])
    print(f"Wrote {package_path}")


if __name__ == "__main__":
    main()
