#!/usr/bin/env python3

import os
import sys
import shutil
import subprocess
import datetime
from pathlib import Path
from typing import List, Optional
import argparse

IMAGE_NAME = "Cervus"
VERSION = "v0.0.1"
QEMUFLAGS = "-m 2G"

BASE_DIR = Path.cwd()
KERNEL_DIR = BASE_DIR / "kernel"
SRC_DIR = KERNEL_DIR / "src"
LINKER_SCRIPTS_DIR = KERNEL_DIR / "linker-scripts"
LINKER_SCRIPT = LINKER_SCRIPTS_DIR / "x86_64.lds"
BIN_DIR = BASE_DIR / "bin"
OBJ_DIR = BASE_DIR / "obj"
ISO_ROOT = BASE_DIR / "iso_root"
DEMO_ISO_DIR = BASE_DIR / "demo_iso"
LIMINE_TOOLS_DIR = BASE_DIR / "limine-tools"

DEPENDENCIES = {
    "freestnd-c-hdrs": {
        "url": "https://codeberg.org/OSDev/freestnd-c-hdrs-0bsd.git",
        "commit": "5df91dd7062ad0c54f5ffd86193bb9f008677631"
    },
    "cc-runtime": {
        "url": "https://codeberg.org/OSDev/cc-runtime.git",
        "commit": "dae79833b57a01b9fd3e359ee31def69f5ae899b"
    },
    "limine-protocol": {
        "url": "https://codeberg.org/Limine/limine-protocol.git",
        "commit": "c4616df2572d77c60020bdefa617dd9bdcc6566a"
    }
}

class Colors:
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BLUE = '\033[94m'
    END = '\033[0m'
    BOLD = '\033[1m'

def print_color(color: str, message: str):
    print(f"{color}{message}{Colors.END}")

def print_help():
    help_text = f"""
{Colors.BOLD}{Colors.GREEN}OS Build Script{Colors.END}
{Colors.BOLD}================{Colors.END}

{Colors.BOLD}Usage:{Colors.END}
  python build.py [command] [options]

{Colors.BOLD}Commands:{Colors.END}
  {Colors.GREEN}run{Colors.END}      - Build and run in QEMU (auto-rebuild if needed)
  {Colors.GREEN}clean{Colors.END}    - Full cleanup (removes all build files and dependencies)
  {Colors.GREEN}cleaniso{Colors.END} - Clean demo_iso directory only
  {Colors.GREEN}gitclean{Colors.END} - Remove all generated files (demo_iso, limine.conf, limine-tools, limine, linker-scripts)

{Colors.BOLD}Options:{Colors.END}
  {Colors.BLUE}--tree{Colors.END}    - Generate OS-TREE.txt with directory structure and file contents
  {Colors.BLUE}--help{Colors.END}    - Show this help message

{Colors.BOLD}Examples:{Colors.END}
  python build.py run
  python build.py clean
  python build.py cleaniso
  python build.py gitclean
  python build.py --tree
  python build.py run --tree
    """
    print(help_text)

def run_command(cmd: List[str], cwd: Path = None, check: bool = True) -> bool:
    print_color(Colors.BLUE, f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=check, 
                               capture_output=True, text=True)
        if result.returncode != 0 and check:
            print_color(Colors.RED, f"Error: {result.stderr}")
            return False
        return True
    except Exception as e:
        print_color(Colors.RED, f"Command failed: {e}")
        return False

def ensure_linker_scripts():
    print_color(Colors.GREEN, "Checking linker scripts...")
    
    LINKER_SCRIPTS_DIR.mkdir(parents=True, exist_ok=True)
    
    if not LINKER_SCRIPT.exists():
        print_color(Colors.YELLOW, "Creating x86_64.lds...")
        
        linker_content = """OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kernel_main)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .note.gnu.build-id : {
        *(.note.gnu.build-id)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
"""
        LINKER_SCRIPT.write_text(linker_content)
        print_color(Colors.GREEN, "x86_64.lds created")
    
    main_c = KERNEL_DIR / "src" / "main.c"
    if main_c.exists():
        content = main_c.read_text()
        if "kmain" in content and "kernel_main" not in content:
            print_color(Colors.YELLOW, "Warning: main.c uses 'kmain' but linker expects 'kernel_main'")
            print_color(Colors.YELLOW, "Consider renaming kmain to kernel_main or updating linker script")
    
    return True

def _is_text_file(filepath: Path) -> bool:
    text_extensions = {'.c', '.h', '.S', '.asm', '.lds', '.conf', 
                       '.py', '.sh', '.txt', '.md', '.rst', '.yml', '.yaml',
                       'Makefile', 'GNUmakefile', 'Dockerfile', '.gitignore'}
    
    file_ext = filepath.suffix.lower()
    if file_ext in text_extensions:
        return True
    
    filename_lower = filepath.name.lower()
    if filename_lower in text_extensions:
        return True
    
    if 'makefile' in filename_lower:
        return True
    
    try:
        with open(filepath, 'rb') as f:
            content = f.read(1024)
            text_chars = bytearray({7, 8, 9, 10, 12, 13, 27} | set(range(0x20, 0x100)) - {0x7f})
            return not bool(content.translate(None, text_chars))
    except:
        return False

def _find_files_by_patterns(patterns: List[str]) -> List[Path]:
    import fnmatch
    files = []
    
    for pattern in patterns:
        for root, dirnames, filenames in os.walk(BASE_DIR):
            dirnames[:] = [d for d in dirnames if not d.startswith('.')]
            
            for filename in filenames:
                if fnmatch.fnmatch(filename, pattern):
                    filepath = Path(root) / filename
                    files.append(filepath)
    
    return files

def generate_tree(output_file: str = "OS-TREE.txt", 
                 specific_files: Optional[List[str]] = None,
                 structure_only: bool = False) -> bool:
    print_color(Colors.GREEN, f"Generating tree in {output_file}...")
    
    if specific_files:
        print_color(Colors.YELLOW, f"Showing only selected files: {specific_files}")
        target_files = _find_files_by_patterns(specific_files)
        if not target_files:
            print_color(Colors.RED, f"No files found for patterns: {specific_files}")
            return False
        print_color(Colors.BLUE, f"Found {len(target_files)} files")
    else:
        target_files = None

    with open(output_file, "w", encoding="utf-8") as out:
        root = Path(".")

        skip_dirs = {
            "bin", "obj", "demo_iso", "iso_root",
            "cc-runtime", "freestnd-c-hdrs",
            "limine-protocol", "edk2-ovmf", "limine"
        }

        exclude_dirs = {"__pycache__", ".git", ".vscode", ".idea", "node_modules"}
        exclude_files = {"*.o", "*.d", "*.fd", "*.iso", "*.hdd", "*.pyc", "*.so", "*.a"}

        def should_exclude(p: Path) -> bool:
            if any(part.startswith('.') for part in p.parts):
                return True
            
            if any(excl in str(p) for excl in exclude_dirs):
                return True
            
            import fnmatch
            if any(fnmatch.fnmatch(p.name, pattern) for pattern in exclude_files):
                return True
            
            return False

        def write_tree(path: Path, prefix: str = "", is_last: bool = True):
            connector = "└── " if is_last else "├── "
            out.write(f"{prefix}{connector}{path.name}\n")

            if path.is_dir():
                if path.name in skip_dirs:
                    out.write(f"{prefix}    └── <скрыто: системная/бинарная директория>\n")
                    return
                
                children = sorted([p for p in path.iterdir() if not should_exclude(p)])
                
                if target_files:
                    relevant_children = []
                    for child in children:
                        if child.is_dir():
                            if any(target_file.is_relative_to(child) for target_file in target_files):
                                relevant_children.append(child)
                        else:
                            if child in target_files:
                                relevant_children.append(child)
                    children = relevant_children
                
                for i, child in enumerate(children):
                    extension = "    " if is_last else "│   "
                    write_tree(child, prefix + extension, i == len(children) - 1)
            else:
                if not structure_only and _is_text_file(path):
                    try:
                        out.write(f"{prefix}    │\n")
                        out.write(f"{prefix}    ├── CONTENT:\n")
                        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read().strip()
                            lines = content.splitlines() or ["<empty>"]
                            for line in lines[:100]:
                                out.write(f"{prefix}    │   {line}\n")
                            if len(lines) > 100:
                                out.write(f"{prefix}    │   ... ({len(lines) - 100} more lines)\n")
                        out.write(f"{prefix}    │\n")
                    except Exception as e:
                        out.write(f"{prefix}    │   <error reading: {e}>\n")

        out.write(f"OS Project: {IMAGE_NAME} {VERSION}\n")
        out.write(f"Generated: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        
        if structure_only:
            out.write("Mode: Structure only\n")
        elif specific_files:
            out.write(f"Mode: Selected files ({', '.join(specific_files)})\n")
        else:
            out.write("Mode: Full tree with text file contents\n")
        
        out.write("=" * 80 + "\n\n")
        
        write_tree(root)

    print_color(Colors.GREEN, f"Tree saved to {output_file}")
    if specific_files:
        print_color(Colors.BLUE, f"Showed {len(target_files)} files")
    
    return True

def check_limine_tools():
    if not LIMINE_TOOLS_DIR.exists():
        print_color(Colors.YELLOW, "limine-tools directory not found")
        return False
    
    for dep_name in DEPENDENCIES:
        dep_dir = LIMINE_TOOLS_DIR / dep_name
        if not dep_dir.exists():
            print_color(Colors.YELLOW, f"Dependency {dep_name} not found in limine-tools")
            return False
        
        git_dir = dep_dir / ".git"
        if not git_dir.exists():
            print_color(Colors.YELLOW, f"Dependency {dep_name} is not a git repository")
            return False
    
    return True

def setup_missing_dependencies():
    print_color(Colors.GREEN, "Checking dependencies...")
    
    LIMINE_TOOLS_DIR.mkdir(exist_ok=True)
    
    setup_needed = False
    for dep_name, dep_info in DEPENDENCIES.items():
        dep_dir = LIMINE_TOOLS_DIR / dep_name
        if not dep_dir.exists():
            print_color(Colors.YELLOW, f"Missing {dep_name}, setting up...")
            setup_needed = True
            
            if not run_command(["git", "clone", dep_info["url"], str(dep_dir)]):
                return False
            
            if not run_command(["git", "-c", "advice.detachedHead=false", 
                              "checkout", dep_info["commit"]], cwd=dep_dir):
                return False
    
    if setup_needed:
        print_color(Colors.GREEN, "Missing dependencies installed")
    else:
        print_color(Colors.GREEN, "All dependencies already exist")
    
    return True

def check_limine():
    limine_dir = BASE_DIR / "limine"
    if not limine_dir.exists():
        print_color(Colors.YELLOW, "Limine directory not found")
        return False
    
    limine_binary = limine_dir / "limine"
    if not limine_binary.exists():
        print_color(Colors.YELLOW, "Limine binary not found")
        return False
    
    return True

def build_limine():
    limine_dir = BASE_DIR / "limine"
    
    if limine_dir.exists():
        limine_binary = limine_dir / "limine"
        if limine_binary.exists():
            print_color(Colors.GREEN, "Limine already built")
            return True
        else:
            print_color(Colors.YELLOW, "Limine directory exists but binary not found, rebuilding...")
            shutil.rmtree(limine_dir)
    
    print_color(Colors.GREEN, "Building Limine...")
    
    if not run_command(["git", "clone", "https://codeberg.org/Limine/Limine.git", 
                       "limine", "--branch=v10.x-binary", "--depth=1"]):
        return False
    
    if not run_command(["make"], cwd=limine_dir):
        return False
    
    return True

def ensure_limine_conf():
    limine_conf = BASE_DIR / "limine.conf"
    if not limine_conf.exists():
        print_color(Colors.YELLOW, "Creating limine.conf...")
        content = f"""# Timeout in seconds
timeout: 3

# Entry name
/{IMAGE_NAME} {VERSION}
    protocol: limine
    path: boot():/boot/kernel
"""
        limine_conf.write_text(content)
        print_color(Colors.GREEN, "limine.conf created")
    
    return True

def find_kernel_source_files():
    source_files = []
    
    if KERNEL_DIR.exists():
        for root, _, files in os.walk(KERNEL_DIR / "src"):
            if '.git' in root:
                continue
            
            for file in files:
                if file.endswith(('.c', '.S', '.asm')):
                    source_files.append(Path(root) / file)
    
    return source_files

def compile_kernel():
    print_color(Colors.GREEN, "Compiling kernel...")
    
    if not ensure_linker_scripts():
        return False
    
    if not check_limine_tools():
        print_color(Colors.RED, "limine-tools not found or incomplete")
        return False
    
    cflags = "-g -O2 -pipe -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding " \
             "-fno-stack-protector -fno-stack-check -fno-lto -fno-PIC " \
             "-ffunction-sections -fdata-sections -m64 -march=x86-64 " \
             "-mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 " \
             "-mno-red-zone -mcmodel=kernel"
    
    cppflags = f"-I {KERNEL_DIR / 'src'} " \
               f"-I {LIMINE_TOOLS_DIR / 'limine-protocol/include'} " \
               f"-isystem {LIMINE_TOOLS_DIR / 'freestnd-c-hdrs/include'} " \
               "-MMD -MP"
    
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    OBJ_DIR.mkdir(parents=True, exist_ok=True)
    
    source_files = find_kernel_source_files()
    object_files = []
    
    for src_file in source_files:
        rel_path = src_file.relative_to(KERNEL_DIR)
        obj_name = str(rel_path).replace(os.sep, '_').replace('/', '_').replace('.', '_')
        obj_file = OBJ_DIR / f"{obj_name}.o"
        obj_file.parent.mkdir(parents=True, exist_ok=True)
        
        if obj_file.exists():
            src_mtime = src_file.stat().st_mtime
            obj_mtime = obj_file.stat().st_mtime
            if src_mtime <= obj_mtime:
                print_color(Colors.BLUE, f"Using cached: {src_file.name}")
                object_files.append(obj_file)
                continue
        
        print_color(Colors.BLUE, f"Compiling {src_file.name}")
        
        if src_file.suffix == '.c':
            cmd = ["gcc"] + cflags.split() + cppflags.split() + \
                  ["-c", str(src_file), "-o", str(obj_file)]
        elif src_file.suffix == '.S':
            cmd = ["gcc"] + cflags.split() + cppflags.split() + \
                  ["-c", str(src_file), "-o", str(obj_file)]
        elif src_file.suffix == '.asm':
            cmd = ["nasm", "-g", "-F", "dwarf", "-f", "elf64", str(src_file), 
                  "-o", str(obj_file)]
        else:
            continue
        
        if not run_command(cmd):
            return False
        
        object_files.append(obj_file)
    
    kernel_output = BIN_DIR / "kernel"
    link_needed = True
    
    if kernel_output.exists() and object_files:
        kernel_mtime = kernel_output.stat().st_mtime
        for obj_file in object_files:
            if obj_file.stat().st_mtime > kernel_mtime:
                link_needed = True
                break
        else:
            link_needed = False
    
    if link_needed:
        print_color(Colors.BLUE, "Linking kernel...")
        
        ldflags = f"-m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 " \
                  f"--gc-sections -T {LINKER_SCRIPT}"
        
        cmd = ["ld"] + ldflags.split() + [str(obj) for obj in object_files] + \
              ["-o", str(kernel_output)]
        
        if not run_command(cmd):
            return False
    else:
        print_color(Colors.GREEN, "Kernel is up to date")
    
    print_color(Colors.GREEN, f"Kernel ready: {kernel_output}")
    return True

def create_iso():
    print_color(Colors.GREEN, "Creating ISO...")
    
    if not check_limine():
        print_color(Colors.RED, "Limine not built")
        return False
    
    kernel_binary = BIN_DIR / "kernel"
    if not kernel_binary.exists():
        print_color(Colors.RED, "Kernel not built")
        return False
    
    if ISO_ROOT.exists():
        shutil.rmtree(ISO_ROOT)
    
    (ISO_ROOT / "boot" / "limine").mkdir(parents=True, exist_ok=True)
    (ISO_ROOT / "EFI" / "BOOT").mkdir(parents=True, exist_ok=True)
    
    print_color(Colors.BLUE, "Copying files...")
    shutil.copy(kernel_binary, ISO_ROOT / "boot")
    ensure_limine_conf()
    shutil.copy(BASE_DIR / "limine.conf", ISO_ROOT / "boot" / "limine")
    
    limine_dir = BASE_DIR / "limine"
    shutil.copy(limine_dir / "limine-bios.sys", ISO_ROOT / "boot" / "limine")
    shutil.copy(limine_dir / "limine-bios-cd.bin", ISO_ROOT / "boot" / "limine")
    shutil.copy(limine_dir / "limine-uefi-cd.bin", ISO_ROOT / "boot" / "limine")
    shutil.copy(limine_dir / "BOOTX64.EFI", ISO_ROOT / "EFI" / "BOOT")
    shutil.copy(limine_dir / "BOOTIA32.EFI", ISO_ROOT / "EFI" / "BOOT")
    
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    iso_filename = f"{IMAGE_NAME}.{VERSION}.{timestamp}.iso"
    
    DEMO_ISO_DIR.mkdir(exist_ok=True)
    iso_path = DEMO_ISO_DIR / iso_filename
    
    print_color(Colors.BLUE, f"Creating ISO: {iso_path}")
    
    cmd = [
        "xorriso", "-as", "mkisofs", "-R", "-r", "-J",
        "-b", "boot/limine/limine-bios-cd.bin",
        "-no-emul-boot", "-boot-load-size", "4", "-boot-info-table", "-hfsplus",
        "-apm-block-size", "2048", "--efi-boot", "boot/limine/limine-uefi-cd.bin",
        "-efi-boot-part", "--efi-boot-image", "--protective-msdos-label",
        str(ISO_ROOT), "-o", str(iso_path)
    ]
    
    if not run_command(cmd):
        return False
    
    print_color(Colors.BLUE, "Installing Limine BIOS...")
    if not run_command([str(limine_dir / "limine"), "bios-install", str(iso_path)]):
        return False
    
    shutil.rmtree(ISO_ROOT)
    
    latest_iso = DEMO_ISO_DIR / f"{IMAGE_NAME}.latest.iso"
    if latest_iso.exists():
        latest_iso.unlink()
    latest_iso.symlink_to(iso_path)
    
    print_color(Colors.GREEN, f"ISO created: {iso_path}")
    print_color(Colors.GREEN, f"Latest ISO: {latest_iso}")
    return True

def run_qemu():
    print_color(Colors.GREEN, "Starting QEMU...")
    
    latest_iso = DEMO_ISO_DIR / f"{IMAGE_NAME}.latest.iso"
    if not latest_iso.exists():
        iso_files = list(DEMO_ISO_DIR.glob("*.iso"))
        if not iso_files:
            print_color(Colors.RED, "No ISO found. Building first...")
            if not build_all():
                return False
            latest_iso = DEMO_ISO_DIR / f"{IMAGE_NAME}.latest.iso"
        else:
            latest_iso = iso_files[-1]
    
    qemu_cmd = [
        "qemu-system-x86_64", "-M", "q35",
        "-cdrom", str(latest_iso),
        "-boot", "d"
    ]
    
    if QEMUFLAGS:
        qemu_cmd += QEMUFLAGS.split()
    
    print_color(Colors.BLUE, f"Running: {' '.join(qemu_cmd)}")
    return run_command(qemu_cmd, check=False)

def build_all():
    print_color(Colors.GREEN, "Building everything...")
    
    DEMO_ISO_DIR.mkdir(exist_ok=True)
    
    if not ensure_linker_scripts():
        return False
    
    if not setup_missing_dependencies():
        return False
    
    if not build_limine():
        return False
    
    if not compile_kernel():
        return False
    
    if not create_iso():
        return False
    
    return True

def incremental_build():
    print_color(Colors.GREEN, "Incremental build...")
    
    if not ensure_linker_scripts():
        return False
    
    if not check_limine_tools():
        print_color(Colors.YELLOW, "Some dependencies missing, setting up...")
        if not setup_missing_dependencies():
            return False
    else:
        print_color(Colors.GREEN, "All dependencies available")
    
    if not check_limine():
        print_color(Colors.YELLOW, "Limine not built, building...")
        if not build_limine():
            return False
    else:
        print_color(Colors.GREEN, "Limine already built")
    
    if not compile_kernel():
        return False
    
    if not create_iso():
        return False
    
    return True

def clean():
    print_color(Colors.GREEN, "Performing full cleanup...")
    
    dirs_to_remove = [
        BIN_DIR,
        OBJ_DIR,
        ISO_ROOT,
        BASE_DIR / "limine",
        LIMINE_TOOLS_DIR,
        BASE_DIR / "edk2-ovmf"
    ]
    
    for dir_path in dirs_to_remove:
        if dir_path.exists():
            print_color(Colors.BLUE, f"Removing {dir_path}")
            shutil.rmtree(dir_path)
    
    files_to_remove = [
        BASE_DIR / f"{IMAGE_NAME}.iso",
        BASE_DIR / f"{IMAGE_NAME}.hdd",
        KERNEL_DIR / ".deps-obtained",
        BASE_DIR / "OS-TREE.txt"
    ]
    
    for file_path in files_to_remove:
        if file_path.exists():
            print_color(Colors.BLUE, f"Removing {file_path}")
            file_path.unlink()
    
    print_color(Colors.GREEN, "Cleanup complete!")
    return True

def cleaniso():
    print_color(Colors.GREEN, "Cleaning demo_iso directory...")
    
    if DEMO_ISO_DIR.exists():
        print_color(Colors.BLUE, f"Removing {DEMO_ISO_DIR}")
        shutil.rmtree(DEMO_ISO_DIR)
        DEMO_ISO_DIR.mkdir(exist_ok=True)
        print_color(Colors.GREEN, "demo_iso directory cleaned!")
    else:
        print_color(Colors.YELLOW, "demo_iso directory doesn't exist")
    
    return True

def gitclean():
    print_color(Colors.GREEN, "Removing all generated files...")
    
    items_to_remove = [
        DEMO_ISO_DIR,
        BASE_DIR / "limine.conf",
        LIMINE_TOOLS_DIR,
        BASE_DIR / "limine",
        LINKER_SCRIPTS_DIR,
        BIN_DIR,
        OBJ_DIR,
        ISO_ROOT,
    ]
    
    for item_path in items_to_remove:
        if item_path.exists():
            print_color(Colors.BLUE, f"Removing: {item_path}")
            if item_path.is_dir():
                shutil.rmtree(item_path)
            else:
                item_path.unlink()
        else:
            print_color(Colors.YELLOW, f"Doesn't exist: {item_path}")
    
    print_color(Colors.GREEN, "gitclean complete!")
    return True

def should_rebuild() -> bool:
    if not check_limine_tools():
        print_color(Colors.YELLOW, "Missing dependencies")
        return True
    
    if not check_limine():
        print_color(Colors.YELLOW, "Limine not built")
        return True
    
    if not LINKER_SCRIPT.exists():
        print_color(Colors.YELLOW, "Linker script not found")
        return True
    
    kernel_binary = BIN_DIR / "kernel"
    if not kernel_binary.exists():
        print_color(Colors.YELLOW, "Kernel not built")
        return True
    
    kernel_mtime = kernel_binary.stat().st_mtime
    source_files = find_kernel_source_files()
    
    for src_file in source_files:
        if src_file.stat().st_mtime > kernel_mtime:
            print_color(Colors.YELLOW, f"Source file {src_file.name} is newer than kernel")
            return True
    
    latest_iso = DEMO_ISO_DIR / f"{IMAGE_NAME}.latest.iso"
    if not latest_iso.exists():
        print_color(Colors.YELLOW, "No latest ISO found")
        return True
    
    print_color(Colors.GREEN, "Everything is up to date")
    return False

def main():
    parser = argparse.ArgumentParser(description="OS Build Script", add_help=False)
    parser.add_argument("command", nargs="?", help="Command to execute")
    parser.add_argument("--tree", action="store_true", help="Generate OS-TREE.txt")
    parser.add_argument("--help", "-h", action="store_true", help="Show help message")
    parser.add_argument("--files", nargs="+", help="Specific files to include in tree")
    parser.add_argument("--structure-only", action="store_true", 
                       help="Generate tree structure only (no file contents)")
    
    args = parser.parse_args()
    
    if args.help or (not args.command and not args.tree and not args.files):
        print_help()
        return 0
    
    if args.tree or args.files:
        specific_files = args.files if args.files else None
        structure_only = args.structure_only
        
        if not generate_tree(specific_files=specific_files, structure_only=structure_only):
            return 1
        
        if not args.command:
            return 0
    
    command = args.command.lower() if args.command else ""
    
    if command == "run":
        if should_rebuild():
            print_color(Colors.YELLOW, "Incremental rebuild...")
            if not incremental_build():
                return 1
        else:
            print_color(Colors.GREEN, "Everything is up to date")
        
        return 0 if run_qemu() else 1
    
    elif command == "clean":
        return 0 if clean() else 1
    
    elif command == "cleaniso":
        return 0 if cleaniso() else 1
    
    elif command == "gitclean":
        return 0 if gitclean() else 1
    
    else:
        print_color(Colors.RED, f"Unknown command: {command}")
        print_help()
        return 1

if __name__ == "__main__":
    sys.exit(main())