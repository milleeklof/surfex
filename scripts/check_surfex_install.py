#!/usr/bin/env python3

"""List Python interpreters on PATH and whether they can import Surfex.

Usage:
  python scripts/check_surfex_install.py
  python scripts/check_surfex_install.py /path/to/python /other/python
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class PythonInfo:
    path: str
    version: str
    surfex_ok: bool
    surfex_file: str | None


def candidate_interpreters() -> list[str]:
    if len(sys.argv) > 1:
        return [str(Path(arg).expanduser()) for arg in sys.argv[1:]]

    names: list[str] = []
    seen: set[str] = set()

    def add(path: str | None) -> None:
        if not path:
            return
        real = os.path.realpath(path)
        if real in seen:
            return
        seen.add(real)
        names.append(path)

    add(sys.executable)
    for name in ("python", "python3"):
        add(shutil.which(name))

    for entry in os.environ.get("PATH", "").split(os.pathsep):
        if not entry:
            continue
        p = Path(entry)
        if not p.is_dir():
            continue
        for child in p.iterdir():
            name = child.name.lower()
            if not name.startswith("python"):
                continue
            if not re.fullmatch(r"python(\d+(?:\.\d+)*)?(m|w)?(\.exe)?", name):
                continue
            if not os.access(child, os.X_OK) or child.is_dir():
                continue
            add(str(child))

    return names


def inspect_python(path: str) -> PythonInfo:
    code = (
        "import sys\n"
        "try:\n"
        "    import surfex\n"
        "    print('OK')\n"
        "    print(getattr(surfex, '__file__', ''))\n"
        "except Exception as exc:\n"
        "    print('ERR')\n"
        "    print(type(exc).__name__ + ': ' + str(exc))\n"
    )

    try:
        result = subprocess.run(
            [path, "-c", code],
            check=False,
            capture_output=True,
            text=True,
            timeout=10,
        )
    except Exception as exc:
        return PythonInfo(path=path, version="unknown", surfex_ok=False, surfex_file=None)

    version = "unknown"
    try:
        version_result = subprocess.run(
            [path, "-c", "import sys; print(sys.version.split()[0])"],
            check=False,
            capture_output=True,
            text=True,
            timeout=10,
        )
        version = version_result.stdout.strip() or "unknown"
    except Exception:
        pass

    stdout = result.stdout.splitlines()
    if stdout and stdout[0].strip() == "OK":
        return PythonInfo(
            path=path,
            version=version,
            surfex_ok=True,
            surfex_file=stdout[1].strip() if len(stdout) > 1 else None,
        )

    return PythonInfo(path=path, version=version, surfex_ok=False, surfex_file=None)


def main() -> int:
    interpreters = candidate_interpreters()
    if not interpreters:
        print("No Python interpreters found.")
        return 1

    print("Checking for your Python environments...")
    print("-" * 80)
    for path in interpreters:
        info = inspect_python(path)
        status = "YES" if info.surfex_ok else "NO"
        print(f"{status}  {info.path}  (Python {info.version})")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
