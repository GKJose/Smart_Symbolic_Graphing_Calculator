from posixpath import splitdrive
import sys
import re
import platform
import os
import argparse
import json
import hashlib
from concurrent.futures.thread import ThreadPoolExecutor

#args = sys.argv[1:]

class TempArgs:
    option = "make"
    threads = 4
    joball = False
    

def btn(val):
    if val:
        return 1
    return 0

def getListOfFiles(dirName, suffix):
    # create a list of file and sub directories 
    # names in the given directory 
    listOfFile = os.listdir(dirName)
    allFiles = list()
    # Iterate over all the entries
    for entry in listOfFile:
        # Create full path
        fullPath = os.path.join(dirName, entry)
        
        # If entry is a directory then get the list of files in this directory 
        if os.path.isdir(fullPath):
            allFiles = allFiles + getListOfFiles(fullPath, suffix)
        elif re.search(re.compile(f".{suffix}$"), fullPath):
            allFiles.append((fullPath, entry))
                
    return allFiles        


def getListOfFiles2(dirName, suffix):
    # create a list of file and sub directories 
    # names in the given directory 
    listOfFile = os.listdir(dirName)
    allFiles = list()
    # Iterate over all the entries
    for entry in listOfFile:
        # Create full path
        fullPath = os.path.join(dirName, entry)

        if not os.path.isdir(fullPath) and re.search(re.compile(f"\.{suffix}$"), fullPath):
            allFiles.append(fullPath)
                
    return allFiles

def sync_system(cmd, src):
    print(f"CC {src}")
    os.system(cmd)


def precompiled_filter(srcs):
    CHUNK_SIZE = 65535
    srcs_data = {}
    file_data = {}
    new_list = []
    for src in srcs:
        md5_hash = hashlib.md5()
        with open(src[0], 'r', encoding='utf-8') as f:
            while True:
                data = f.read(CHUNK_SIZE).encode('utf-8')
                if not data:
                    break
                md5_hash.update(data)

            srcs_data[src[0]] = md5_hash.hexdigest()
    try:
        with open('.pclist', 'r') as f:
            try: 
                file_data = json.loads(''.join(f.readlines()))
            except json.decoder.JSONDecodeError:
                ...
    except:
        ...
    
    for src in srcs:
        if src[0] not in file_data.keys():
            file_data[src[0]] = srcs_data[src[0]]
            new_list.append(src)
        else:
            if file_data[src[0]] != srcs_data[src[0]]:
                file_data[src[0]] = srcs_data[src[0]]
                new_list.append(src)
    with open('.pclist', 'w') as f:
        f.write(json.dumps(file_data))
    return new_list




def main():
    plat = platform.platform()
    is_pi = re.search(r'armv6', plat) != None
    is_linux = (platform.system() == 'Linux') and not is_pi
    is_windows = platform.system() == 'Windows'
    enable_giac = False
    enable_sdl2 = is_linux or is_windows
    executable_suffix = "" if is_linux or is_pi else ".exe"
    object_suffix = "o"

    parser = argparse.ArgumentParser()
    parser.add_argument("option", help="Supports 'init', 'make', and 'clean'.")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-t", "--threads", help="Sets the number of threads used during compilation.", type=int, default=1)
    group.add_argument("-j", "--joball", help="Sets the total number of threads used during compilation to the maximum amount available.", action="store_true")
    args = parser.parse_args()
    #args = TempArgs()
    num_threads = args.threads if not args.joball else os.cpu_count()

    pwd = os.path.dirname(os.path.abspath(__file__))
    # pwd = sys.argv[0]
    if not is_pi and not is_linux and not is_windows:
        print(f"Unsupported system detected. {platform.system()}")
        print("Currently supported systems are Linux and Windows")

    if args.option == "init":
        print("Enable GIAC library? (not reccommended unless building for pi) (yes/no) : ")
        while True:
            v = input()
            if v == 'yes' or v == 'YES':
                enable_giac = True 
                break
            elif v == 'no' or v == 'NO':
                break
            else:
                print("Invalid input. Try again. (yes/no): ")


        calc_conf_template = f"""
        #ifndef SMART_CALC_H
        #define SMART_CALC_H

        #define ENABLE_GIAC {btn(enable_giac)}
        #define ENABLE_PI {btn(is_pi)}
        #define ENABLE_LINUX {btn(is_linux)}
        #define ENABLE_WINDOWS {btn(is_windows)}

        #if ENABLE_WINDOWS
        #define USE_WIN32DRV 1
        #define USE_FBDEV 0
        #define USE_EVDEV 0
        #endif

        #if ENABLE_LINUX
        #define USE_MONITOR 1
        #define USE_MOUSEWHEEL 1
        #define USE_MOUSE 1
        #define USE_KEYBOARD 1
        #endif

        #endif"""

        with open('calc_conf.h', 'w') as f:
            f.write(calc_conf_template)
        # END OF INIT
    elif args.option == "make":

        with open('calc_conf.h', 'r') as f:
            for line in f:
                if re.search(r'#define ENABLE_GIAC 1', line):
                    enable_giac = True

        liblist = ''

        if enable_giac:
            liblist += '-lgiac -lgmp '

        if enable_sdl2:
            liblist += '-lSDL2 '

        if is_linux or is_pi:
            liblist += '-lm -lpthread '

        if is_windows:
            liblist += '-lgdi32 '
        
        csrcs = []
        cxxsrcs = []
        xtra = "-DUNICODE" if is_windows else ""
        cflags = f"-O3 -g0 -I{pwd} -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare {xtra}"
        cxxflags = f"-std=c++14 -O3 -g0 -I{pwd} -Wall {xtra}"

        for file in getListOfFiles(os.path.join(pwd, 'lvgl'), ".c"):
            csrcs.append(file)

        for file in getListOfFiles(os.path.join(pwd, 'lv_drivers'), ".c"):
            csrcs.append(file)

        for file in getListOfFiles(os.path.join(pwd, 'lv_demos'), ".c"):
            csrcs.append(file)

        for file in getListOfFiles(os.path.join(pwd, 'lv_demos'), ".cpp"):
            cxxsrcs.append(file)

        target_csrcs = precompiled_filter(csrcs)
        target_cxxsrcs = precompiled_filter(cxxsrcs)

        cobjs = set([x[1]+".o" for x in csrcs])
        cxxobjs = set([x[1]+".o" for x in cxxsrcs])

        cargs = [(f"gcc {cflags} -c {src[0]} -o {src[1]}.{object_suffix} ", src[1]) for src in target_csrcs]
        cxxargs = [(f"g++ {cxxflags} -c {src[0]} -o {src[1]}.{object_suffix}", src[1]) for src in target_cxxsrcs]
        
        


        if num_threads == 1:
            for arg in cargs:
                sync_system(arg[0], arg[1])

            for arg in cxxargs:
                sync_system(arg[0], arg[1])
        else:
            with ThreadPoolExecutor(max_workers=num_threads) as executor:
                for arg in cargs:
                    executor.submit(sync_system, arg[0], arg[1])
                for arg in cxxargs:
                    executor.submit(sync_system, arg[0], arg[1])

            
        
        string = f"g++ {cxxflags} -c main.cpp -o main.{object_suffix}"
        print(string)
        os.system(string)

        with open("target.objs", 'w') as f:
            f.write(' '.join(cobjs) + ' ' + ' '.join(cxxobjs) + f' main.{object_suffix}')

        string = f"g++ {cxxflags} @target.objs {liblist} -o demo{executable_suffix}"
        print(string)
        os.system(string)
        os.remove("target.objs")
        
        # END OF MAKE
    elif args.option == "clean":
        files = getListOfFiles2(pwd, object_suffix)
        for file in files:
            os.remove(file)
        if os.path.exists(os.path.join(pwd, 'demo' + executable_suffix )):
            os.remove(os.path.join(pwd, 'demo' + executable_suffix))
        if os.path.exists(os.path.join(pwd, '.pclist')):
            os.remove(os.path.join(pwd, '.pclist'))
    else:
        print("Unknown argument.")
        print("Valid arguments are: init, make, clean.")

main()