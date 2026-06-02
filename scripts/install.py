#!/usr/bin/env python3

from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

from installer_common import (
    discover_pythons,
    externally_managed,
    module_installed,
    pip_install_pybind11,
    python_version,
    resolve_path,
    run,
    site_packages_path,
    status_text,
    surfex_location,
    writable,
)


REPO_ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent
BUILD_DIR = REPO_ROOT / "build"
VENV_DIR = REPO_ROOT / ".venv"


def prompt_choice(valid: set[str]) -> str:
    while True:
        try:
            value = input("> ").strip()
        except EOFError:
            print("\nInstallation cancelled.")
            raise SystemExit(0)
        lowered = value.lower()
        if lowered in {"q", "quit"}:
            print("Installation cancelled.")
            raise SystemExit(0)
        if value in valid:
            return value
        print(f"Enter one of: {', '.join(sorted(valid))}")


def show_interpreters(interpreters: list[str]) -> None:
    active = resolve_path(sys.executable)
    print("Found Python installations:\n")
    for index, python in enumerate(interpreters, start=1):
        print(f"[{index}] {python}")
        print(f"    Python {python_version(python)}")
        if resolve_path(python) == active:
            print("    (recommended)")
        print(f"    {status_text(module_installed(python, 'surfex'))}")
        print()


def clear_build_dir_if_needed(python: str) -> None:
    cache = BUILD_DIR / "CMakeCache.txt"
    if not cache.is_file():
        return
    for line in cache.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("Python_EXECUTABLE:") and "=" in line:
            cached = line.split("=", 1)[1].strip()
            if cached and resolve_path(cached) != resolve_path(python):
                shutil.rmtree(BUILD_DIR, ignore_errors=True)
            return


def create_local_venv(base_python: str) -> str:
    venv_python = VENV_DIR / "bin" / "python"
    if not venv_python.exists():
        print("Creating local .venv and continuing...\n")
        run([base_python, "-m", "venv", str(VENV_DIR)])
    return str(venv_python)


def reinstall_prompt() -> bool:
    print("Surfex is already installed.\n")
    print("[1] Reinstall / Update")
    print("[2] Cancel")
    print("[Q] Quit")
    return prompt_choice({"1", "2"}) == "1"


def ensure_pybind11(python: str) -> str | None:
    if module_installed(python, "pybind11"):
        return python

    print("pybind11 is missing in the selected environment. Installing it now...\n")
    ok, output = pip_install_pybind11(python)
    if ok:
        return python

    if externally_managed(python) or "externally managed" in output.lower():
        print("This Python installation does not allow packages to be installed directly.\n")
        print("Options:\n")
        print("[1] Create local .venv and continue")
        print("[2] Choose another interpreter")
        print("[3] Force install with --break-system-packages")
        print("[4] Cancel")
        print("[Q] Quit")
        choice = prompt_choice({"1", "2", "3", "4"})
        if choice == "1":
            venv_python = create_local_venv(python)
            ok, output = pip_install_pybind11(venv_python)
            if ok:
                print("\nTo activate this environment later:\nsource .venv/bin/activate\n")
                return venv_python
            print(output or "Failed to install pybind11 in .venv.", file=sys.stderr)
            return None
        if choice == "2":
            return None
        if choice == "3":
            ok, output = pip_install_pybind11(python, break_system_packages=True)
            if ok:
                return python
            print(output or "Failed to install pybind11 with --break-system-packages.", file=sys.stderr)
            return None
        raise SystemExit(0)

    print(output or "Failed to install pybind11.", file=sys.stderr)
    return None


def ensure_writable_target(python: str) -> tuple[str, bool] | None:
    target = site_packages_path(python)
    if not target:
        print("Could not determine the site-packages directory.", file=sys.stderr)
        return None

    if writable(target):
        return python, False

    print("The selected Python installation requires administrator privileges.\n")
    print(f"Target:\n{target}\n")
    print("Options:\n")
    print("[1] Choose another interpreter")
    print("[2] Create local .venv")
    print("[3] Continue with sudo")
    print("[4] Cancel")
    print("[Q] Quit")
    choice = prompt_choice({"1", "2", "3", "4"})
    if choice == "1":
        return None
    if choice == "2":
        return create_local_venv(python), False
    if choice == "3":
        return python, True
    raise SystemExit(0)


def main() -> int:
    while True:
        interpreters = discover_pythons()
        if not interpreters:
            print("No Python installations were found.", file=sys.stderr)
            return 1

        show_interpreters(interpreters)
        print("[Q] Quit\n")

        if len(interpreters) == 1:
            chosen = interpreters[0]
            print("Only one Python installation was found; using it automatically.\n")
        else:
            print("Choose installation target:")
            choice = prompt_choice({str(i) for i in range(1, len(interpreters) + 1)})
            chosen = interpreters[int(choice) - 1]

        if surfex_location(chosen):
            if not reinstall_prompt():
                print("Installation cancelled.")
                return 0

        chosen_python = ensure_pybind11(chosen)
        if chosen_python is None:
            continue

        writable_choice = ensure_writable_target(chosen_python)
        if writable_choice is None:
            continue
        chosen_python, use_sudo = writable_choice

        chosen_python = ensure_pybind11(chosen_python) or chosen_python
        clear_build_dir_if_needed(chosen_python)
        print(f"Using Python: {chosen_python}\n")
        run(["cmake", "-S", str(REPO_ROOT), "-B", str(BUILD_DIR), f"-DPython_EXECUTABLE={chosen_python}"])
        run(["cmake", "--build", str(BUILD_DIR)])
        if use_sudo:
            run(["sudo", "env", f"PATH={os.environ.get('PATH', '')}", "cmake", "--install", str(BUILD_DIR)])
        else:
            run(["cmake", "--install", str(BUILD_DIR)])

        if Path(chosen_python).resolve().as_posix().startswith(VENV_DIR.resolve().as_posix()):
            print("\nTo use this install later:\nsource .venv/bin/activate\n")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
