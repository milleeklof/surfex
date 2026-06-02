#!/usr/bin/env python3

from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

from installer_common import (
    discover_pythons,
    module_installed,
    python_version,
    resolve_path,
    run,
    site_packages_path,
    status_text,
    surfex_location,
    writable,
)


REPO_ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent


def prompt_choice(valid: set[str]) -> str:
    while True:
        try:
            value = input("> ").strip()
        except EOFError:
            print("\nUninstallation cancelled.")
            raise SystemExit(0)
        lowered = value.lower()
        if lowered in {"q", "quit"}:
            print("Uninstallation cancelled.")
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


def remove_surfex(python: str) -> None:
    target = Path(site_packages_path(python)) / "surfex"
    if not target.exists():
        print("Surfex is not installed in this interpreter.\n")
        return

    print(f"Remove Surfex from:\n\n{python}\n")
    print("[1] Uninstall")
    print("[2] Cancel")
    print("[Q] Quit")
    if prompt_choice({"1", "2"}) == "2":
        print("Uninstallation cancelled.")
        return

    if writable(str(target.parent)):
        shutil.rmtree(target)
    else:
        print("Administrator privileges may be required.\n")
        print(f"Target:\n{target}\n")
        print("[1] Continue with sudo")
        print("[2] Cancel")
        if prompt_choice({"1", "2"}) == "2":
            print("Uninstallation cancelled.")
            return
        run(["sudo", "rm", "-rf", str(target)])

    print("Surfex was successfully removed.")


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
            remove_surfex(chosen)
            return 0

        print("Surfex is not installed in this interpreter.\n")


if __name__ == "__main__":
    raise SystemExit(main())
