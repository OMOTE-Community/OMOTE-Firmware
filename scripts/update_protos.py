"""PlatformIO pre-build script: pulls latest shared protos submodule."""
import subprocess
import os

Import("env")  # noqa: F821 — PlatformIO injects this

project_dir = env.get("PROJECT_DIR", os.getcwd())
submodule_path = os.path.join(project_dir, "protos", "shared")

if os.path.isdir(submodule_path):
    print("Updating protos/shared submodule...")
    result = subprocess.run(
        ["git", "submodule", "update", "--init", "--remote", "protos/shared"],
        cwd=project_dir,
        capture_output=True,
        text=True,
    )
    if result.returncode == 0:
        print("protos/shared is up to date")
    else:
        print(f"Warning: failed to update protos/shared: {result.stderr.strip()}")
else:
    print("protos/shared submodule not found, skipping update")
