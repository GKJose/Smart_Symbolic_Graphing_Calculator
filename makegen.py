import re
import platform
import os
import functools

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
            allFiles.append(fullPath)
                
    return allFiles        


plat = platform.platform()

is_pi = re.search(r'armv6', plat) != None
is_linux = (platform.system() == 'Linux') and not is_pi
is_windows = platform.system() == 'Windows'
enable_giac = False
enable_sdl2 = is_linux or is_windows
pwd = os.path.dirname(os.path.abspath(__file__))

if not is_pi and not is_linux and not is_windows:
    print(f"Unsupported system detected. {platform.system()}")
    print("Currently supported systems are Linux and Windows")
else:
    print("Supported system detected.")


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


calc_conf_template = f"""#ifndef SMART_CALC_H
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

liblist = '-lgmpxx -lgmp'

if enable_giac:
    liblist += '-lgiac -lgmp '

if enable_sdl2:
    liblist += '-lSDL2 '

if is_linux or is_pi:
    liblist += '-lm '

searchdir = lambda p, s: functools.reduce(lambda acc, b: acc+b+' ',getListOfFiles(p, s))
csrcs = ""
cxxsrcs = ""
xtra = "-DUNICODE" if is_windows else ""

for file in getListOfFiles(os.path.join(pwd, 'lvgl'), ".c"):
    csrcs += f"CSRCS += {file}\n"

for file in getListOfFiles(os.path.join(pwd, 'lv_drivers'), ".c"):
    csrcs += f"CSRCS += {file}\n"

for file in getListOfFiles(os.path.join(pwd, 'lv_demos'), ".c"):
    csrcs += f"CSRCS += {file}\n"

for file in getListOfFiles(os.path.join(pwd, 'lv_demos'), ".cpp"):
    csrcs += f"CXXSRCS += {file}\n"

makefile_template = f"""#
# Makefile
#
CC ?= gcc
CXX ?= g++
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= {pwd}
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
CXXFLAGS ?= -std=c++14 -O3 -g0 -I$(LVGL_DIR)/ -Wall {xtra}
LDFLAGS ?= {liblist}
BIN = demo


#Collect the files to compile
MAINSRC = ./main.cpp

CSRCS ?= 
CXXSRCS ?= 

{csrcs}

{cxxsrcs}

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cpp=$(OBJEXT))

MAINOBJ = $(MAINSRC:.cpp=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

%.o: %.cpp
	@$(CXX)  $(CXXFLAGS) -c $< -o $@
	@echo "CXX $<"

default: $(AOBJS) $(COBJS) $(CXXOBJS) $(MAINOBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(CXXOBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(CXXOBJS) $(MAINOBJ)
"""

with open('calc_conf.h', 'w') as f:
    f.write(calc_conf_template)

with open('Makefile', 'w') as f:
    f.write(makefile_template)

print("DONE.")

