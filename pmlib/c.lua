CC = "gcc"
CCFLAGS = { "-Wall", "-c", "-std=c99" }
INCLUDES = EMPTY
COMPILE = "%CC% %CCFLAGS% %INCLUDES% -o %out[1]% %in[1]%"

LD = "%CC%"
LDFLAGS = EMPTY
LIBS = EMPTY
LINK = "%LD% %LDFLAGS% %LIBS% -o %out% %in%"

SOURCEDIR = "src"
INCLUDESDIR = "includes"

cfile = simple {
    class = "cfile";
    command = { "%COMPILE%" };
    outputs = { "%U%-%I%.o" };
}

lib = simple {
    class = "lib";
    command = {
        "ar rcu %out% %in%";
        "ranlib %out%";
    };
    outputs = { "%U%-%I%.a" };
}

sharedlib = simple {
    class = "sharedlib";
    command = { "%LINK%" }; --"%LD% %LDFLAGS% %LIBS% -o %out% %in%" };
    LDFLAGS = { PARENT, "-shared" };
    outputs = { "%U%-%I%.so" };
}

cgen = simple {
    class = "cgen";
    outputs = { "%CDIR%/%I%.c" };
    command = { "%in% %U%-%I%.c %U%-%I%.h" };
    install = {
        pm.install("%U%-%I%.c", "%CDIR%/%I%.c");
        pm.install("%U%-%I%.h", "%HDIR%/%I%.h");
    }   
}

