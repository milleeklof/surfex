from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path


GREEN = "\033[32m"
RED = "\033[31m"
RESET = "\033[0m"


def use_color() -> bool:
    return sys.stdout.isatty()


def color(text: str, code: str) -> str:
    return f"{code}{text}{RESET}" if use_color() else text


def status_text(installed: bool) -> str:
    return color("[SURFEX INSTALLED]", GREEN) if installed else color("[SURFEX NOT INSTALLED]", RED)


def run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True)


def run_capture(cmd: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, check=False, capture_output=True, text=True)


def resolve_path(path: str) -> str:
    return str(Path(path).expanduser().resolve())


def is_python_name(name: str) -> bool:
    return bool(re.fullmatch(r"python(\d+(?:\.\d+)*)?(w)?(?:\.exe)?", name, re.IGNORECASE))


def is_system_python(path: str) -> bool:
    real = resolve_path(path)
    return real.startswith("/usr/bin/") or real.startswith("/System/Library/")


def add_candidate(candidates: list[str], seen: set[str], path: str | os.PathLike[str] | None) -> None:
    if not path:
        return
    candidate = Path(path).expanduser()
    if not candidate.exists() or not candidate.is_file() or not os.access(candidate, os.X_OK):
        return
    if not is_python_name(candidate.name):
        return
    real = resolve_path(str(candidate))
    if real in seen:
        return
    seen.add(real)
    candidates.append(str(candidate))


def discover_pythons() -> list[str]:
    candidates: list[str] = []
    seen: set[str] = set()

    for path in (sys.executable, shutil.which("python3"), shutil.which("python")):
        add_candidate(candidates, seen, path)

    for directory in os.environ.get("PATH", "").split(os.pathsep):
        if not directory:
            continue
        path = Path(directory)
        if not path.is_dir():
            continue
        try:
            for child in sorted(path.iterdir()):
                add_candidate(candidates, seen, child)
        except OSError:
            continue

    for directory in (Path("/opt/homebrew/bin"), Path("/usr/local/bin"), Path("/usr/bin")):
        if not directory.is_dir():
            continue
        try:
            for child in sorted(directory.iterdir()):
                add_candidate(candidates, seen, child)
        except OSError:
            continue

    frameworks = Path("/Library/Frameworks/Python.framework/Versions")
    if frameworks.is_dir():
        for version_dir in sorted(frameworks.iterdir()):
            bin_dir = version_dir / "bin"
            if not bin_dir.is_dir():
                continue
            try:
                for child in sorted(bin_dir.iterdir()):
                    add_candidate(candidates, seen, child)
            except OSError:
                continue

    show_system = os.environ.get("SURFEX_SHOW_SYSTEM_PYTHON") == "1"
    non_system = [p for p in candidates if not is_system_python(p)]
    if non_system and not show_system:
        candidates = non_system

    non_pythonw = [p for p in candidates if not Path(p).name.lower().startswith("pythonw")]
    if non_pythonw:
        candidates = non_pythonw

    return candidates


def python_version(python: str) -> str:
    result = run_capture([python, "-c", "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"])
    return result.stdout.strip() or "unknown"


def module_installed(python: str, module: str) -> bool:
    code = f"import importlib.util, sys; sys.exit(0 if importlib.util.find_spec('{module}') else 1)"
    return run_capture([python, "-c", code]).returncode == 0


def surfex_location(python: str) -> str:
    code = r"""
import importlib.util
import json
spec = importlib.util.find_spec('surfex')
if spec is None:
    print(json.dumps({'installed': False, 'location': ''}))
else:
    location = getattr(spec, 'origin', '') or ''
    if not location:
        locations = getattr(spec, 'submodule_search_locations', None)
        if locations:
            location = next(iter(locations), '')
    print(json.dumps({'installed': True, 'location': location}))
"""
    result = run_capture([python, "-c", code])
    try:
        data = json.loads(result.stdout.strip() or "{}")
    except json.JSONDecodeError:
        return ""
    return str(data.get("location", "")) if data.get("installed") else ""


def site_packages_path(python: str) -> str:
    code = r"""
import sysconfig
path = sysconfig.get_path('platlib') or sysconfig.get_path('purelib') or ''
print(path)
"""
    result = run_capture([python, "-c", code])
    return result.stdout.strip()


def externally_managed(python: str) -> bool:
    code = r"""
from pathlib import Path
import sysconfig
stdlib = sysconfig.get_path('stdlib')
print('1' if stdlib and (Path(stdlib) / 'EXTERNALLY-MANAGED').exists() else '0')
"""
    result = run_capture([python, "-c", code])
    return result.returncode == 0 and result.stdout.strip() == "1"


def pip_install_pybind11(python: str, break_system_packages: bool = False) -> tuple[bool, str]:
    cmd = [python, "-m", "pip", "install"]
    if break_system_packages:
        cmd.append("--break-system-packages")
    cmd.append("pybind11")
    result = run_capture(cmd)
    output = (result.stdout or "") + ("\n" + result.stderr if result.stderr else "")
    return result.returncode == 0, output.strip()


def writable(path: str) -> bool:
    return Path(path).exists() and os.access(path, os.W_OK)
