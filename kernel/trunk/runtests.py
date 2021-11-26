#!/usr/bin/python3
# Copyright 2021 Magomed Kostoev
# Published under MIT License

import os
import sys
import urllib
from importlib.machinery import SourceFileLoader
from shutil import which
import timeit
import urllib.request
import subprocess
from threading import Thread

sys.path.append('test')
import common

enable_umka = False

def log(s, end = "\n"):
    print(s, end = end, flush = True)

def execute(s, mute = False):
    mute = ">/dev/null" if mute else ""
    code = os.system(f"{s}{mute}")
    if code:
        print(f"Command returned {code}: \"{s}\"")
        exit(-1)

def stage(name, command, mute = False):
    print(f"{name}... ", end = "")
    execute(command, mute = mute)
    print("Done.")

def download(link, path):
    log(f"Downloading {path}... ", end = "")
    urllib.request.urlretrieve(link, path)
    log("Done.")

def tool_exists(name):
    assert(type(name) == str)
    return which(name) != None

def check_tools(tools):
    assert(type(tools) == tuple)
    for name_package_pair in tools:
        assert(type(name_package_pair) == tuple)
        assert(len(name_package_pair) == 2)
        assert(type(name_package_pair[0]) == str)
        assert(type(name_package_pair[1]) == str)
    
    not_exists = []
    for name, package in tools:
        if not tool_exists(name):
            not_exists.append((name, package))
    if len(not_exists) != 0:
        log("Sorry, I can't find some tools:")

        header_name = "Name"
        header_package = "Package (probably)"

        max_name_len = len(header_name)
        max_package_name_len = len(header_package)
        for name, package in not_exists:
            if len(package) > max_package_name_len:
                max_package_name_len = len(package)
            if len(name) > max_name_len:
                max_name_len = len(name)

        def draw_row(name, package):
            log(f" | {name.ljust(max_name_len)} | {package.ljust(max_package_name_len)} |")

        def draw_line():
            log(f" +-{'-' * max_name_len}-+-{'-' * max_package_name_len}-+")

        draw_line()
        draw_row(header_name, header_package)
        draw_line()
        for name, package in not_exists:
            draw_row(name, package)
        draw_line()
        exit()

def prepare_test_img():
    # TODO: Always recompile the kernel (after build system is done?)
    # Get IMG
    if not os.path.exists("kolibri_test.img"):
        if len(sys.argv) == 1:
            download("http://builds.kolibrios.org/eng/data/data/kolibri.img", "kolibri_test.img")
        else:
            builds_eng = sys.argv[1]
            execute(f"cp {builds_eng}/data/data/kolibri.img kolibri_test.img")
    
    # Open the IMG
    with open("kolibri_test.img", "rb") as img:
        img_data = img.read()
    img = common.Floppy(img_data)

    # Remove unuseful folders
    img.delete_path("GAMES")
    img.delete_path("DEMOS")
    img.delete_path("3D")
    
    # Get test kernel
    if not os.path.exists("kernel.mnt.pretest"):
        if len(sys.argv) == 1:
            with open("lang.inc", "w") as lang_inc:
                lang_inc.write("lang fix en\n")
            execute("fasm bootbios.asm bootbios.bin.pretest -dpretest_build=1")
            execute("fasm -m 65536 kernel.asm kernel.mnt.pretest -dpretest_build=1 -ddebug_com_base=0xe9")
        else:
            builds_eng = sys.argv[1]
            execute(f"cp {builds_eng}/data/kernel/trunk/kernel.mnt.pretest kernel.mnt.pretest", mute = True)
    
    # Put the kernel into IMG
    with open("kernel.mnt.pretest", "rb") as kernel_mnt_pretest:
        kernel_mnt_pretest_data = kernel_mnt_pretest.read()
    img.add_file_path("KERNEL.MNT", kernel_mnt_pretest_data)
    img.save("kolibri_test.img")

def collect_tests():
    tests = []

    # Collect tests from test folder (not recursively yet)
    for test_folder in os.listdir("test"):
        test_folder_path = f"test/{test_folder}"
        test_file = f"{test_folder_path}/test.py"

        if not os.path.isdir(test_folder_path):
            continue

        if os.path.exists(test_file):
            tests.append(test_folder_path)
    return tests

def collect_umka_tests():
    tests = []

    for test_file in os.listdir("umka/test"):
        test_file_path = f"umka/test/{test_file}"
        if not test_file.endswith(".t"):
            continue
        if not os.path.isfile(test_file_path):
            continue
        tests.append(test_file_path)
    return tests

def run_tests_serially_thread(test, root_dir):
    test_number = 1
    for test in tests:
        test_dir = f"{root_dir}/{test}"
    
        os.chdir(test_dir)
        print(f"[{test_number}/{len(tests)}] {test}... ", end = "", flush=True)
        start = timeit.default_timer()
        try:
            SourceFileLoader("test", "test.py").load_module().run()
        except common.TestTimeoutException:
            result = "TIMEOUT"
        except common.TestFailureException:
            result = "FAILURE"
        else:
            result = "SUCCESS"
        finish = timeit.default_timer()
        print(f"{result} ({finish - start:.2f} seconds)")
        os.chdir(root_dir)
    
        test_number += 1

def run_tests_serially(tests, root_dir):
    thread = Thread(target = run_tests_serially_thread, args = (tests, root_dir))
    thread.start()
    return thread

def gcc(fin, fout):
    command = f"gcc -m32 -std=c11 -g -O0 -D_FILE_OFFSET_BITS=64 -DNDEBUG -masm=intel -D_POSIX_C_SOURCE=200809L -Iumka -Iumka/linux -fno-pie -c {fin} -o {fout}"
    print(command)
    os.system(command)

def build_umka():
    if not enable_umka:
        return
    if os.path.exists("umka_shell"):
        return
    os.makedirs("umka/build", exist_ok = True)
    sources = ["umka_shell.c", "shell.c", "trace.c", "trace_lbr.c", "vdisk.c", "vnet.c", "lodepng.c", "linux/pci.c", "linux/thread.c", "util.c"]
    sources = [f"umka/{f}" for f in sources]
    for source in sources:
        gcc(source, f"{source}.o")
    objects = " ".join([ f"{s}.o" for s in sources ])
    os.system(f"INCLUDE=\"../../programs/develop/libraries/libcrash/hash\" fasm -dUEFI=1 -dextended_primary_loader=1 -dUMKA=1 umka/umka.asm umka/build/umka.o -s umka/build/umka.fas -m 2000000")
    objects += (" umka/build/umka.o")
    os.system(f"gcc -m32 -no-pie -o umka_shell -static -T umka/umka.ld {objects}")

if __name__ == "__main__":
    root_dir = os.getcwd()

    # Check available tools
    tools = (("qemu-system-i386", "qemu-system-x86"),
             ("fasm", "fasm"))
    check_tools(tools)
    
    prepare_test_img()
    build_umka()
    tests = collect_tests()
    print(collect_umka_tests)
    serial_executor_thread = run_tests_serially(tests, root_dir)
    serial_executor_thread.join()

