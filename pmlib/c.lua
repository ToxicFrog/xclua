local function tablize(t)
    if type(t) ~= "table"
    or t.class then
        return {t}
    end
    return t
end

local function map(f, l)
    local r = {}
    for i,v in ipairs(l) do
        r[i] = f(v)
    end
    return r                        
end                                  

function pm.stringmodifier.prepend_I(rule, str)
    if str == EMPTY then return end
    return map(function(S) return "-I"..S end, tablize(str))
end

function pm.stringmodifier.prepend_l(rule, str)
    if str == EMPTY then return end
    return map(function(S) return "-l"..S end, tablize(str))
end

function pm.stringmodifier.prepend_L(rule, str)
    if str == EMPTY then return end
    return map(function(S) return "-L"..S end, tablize(str))
end

-- settings for compile phase
CC = "gcc"
CCFLAGS = { "-Wall", "-c", "-std=c99" }
CCINCLUDES = EMPTY
CCCOMPILE = "%CC% %CCFLAGS% %CCINCLUDES:prepend_I% -o %out[1]% %in[1]%"

-- settings for link phase
LD = "%CC%"
LDFLAGS = EMPTY;
LDLIBS = EMPTY;
LDPATHS = EMPTY;
LDLINK = "%LD% %LDFLAGS% %LDPATHS:prepend_L% -o %out% %in% %LDLIBS:prepend_l%";

-- begin main library
c = {}

-- one C source file -> one object file
c.file = simple {
    class = "c.file";
    command = { "%CCCOMPILE%" };
    outputs = { "%U%-%I%.o" };
    
    __init = function(self, p)
        if type(p) == "table" and p.class then
            return simple.__init(self, p)
        end
        
        p = tablize(p)
        
        for i,file in ipairs(p) do
            if self:__expand(file) == file then
                p[i] = "%{ local path = self:__index 'CCSOURCES'; return path and path..'/' or '' }%" .. file
            end
        end
        
        return simple.__init(self, p)
    end;
    
	__dependencies = function(self, inputs, outputs)
        local function close(method)
            return function(...)
                return self[method](self, unpack(arg))
            end
        end
                
        local deps = {}
        local includes = map(close "__expand", tablize(self:__index "CCINCLUDES"))

        local function updatedeps(file)
            if deps[file] then return end
            deps[file] = true
            local fin = io.open(file) or self:__error "can't open file for dependency calculation"
            local buf = fin:read("*a")
            fin:close()
            
            string.gsub(buf, '#include%s+["<]([^">]*)[">]',
                function(included)
                    for _,path in ipairs(includes) do
                        local fin = io.open(path.."/"..included)
                        if fin then
                            deps[path.."/"..included] = true
                            fin:close()
                            updatedeps(path.."/"..included)
                            break
                        end
                    end
                end)
        end
        
        -- calculate dependencies
        for _,input in ipairs(map(close "__expand", inputs)) do
            updatedeps(input)
        end
        
        local realdeps = {}
        for k in pairs(deps) do
            table.insert(realdeps, k)
        end
        
        if pm.verbose then
            print("depends ("
                ..table.concat(inputs, " ")
                .."): "
                ..table.concat(realdeps, " "))
        end
        
        return realdeps
    end;
}

-- poly object files -> one static library
c.staticlib = simple {
    class = "c.staticlib";
    command = {
        "ar rcu %out% %in%";
        "ranlib %out%";
    };
    outputs = { "%U%-%I%.a" };
}

-- poly object files -> one shared library
c.sharedlib = simple {
    class = "c.sharedlib";
    command = { "%LDLINK%" };
    outputs = { "%U%-%I%.so" };

    LDFLAGS = { PARENT, "-shared" };
}

c.program = simple {
    class = "c.program";
    command = { "%LDLINK%" };
    outputs = { "%U%-%I%" };
}

c.gen = simple {
    class = "c.gen";
    outputs = { "%CDIR%/%I%.c" };
    command = { "%in% %U%-%I%.c %U%-%I%.h" };
    install = {
        pm.install("%U%-%I%.c", "%CDIR%/%I%.c");
        pm.install("%U%-%I%.h", "%HDIR%/%I%.h");
    }   
}

