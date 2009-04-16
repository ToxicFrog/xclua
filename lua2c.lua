argv = { ... }
name = argv[1] or 'data'

io.stdout:write('static unsigned char '..name..'_buffer[] = {\n');

fstr = io.stdin:read("*a")

for i=1,string.len(fstr) do
	col = math.mod(i, 8)
	if col == 1 then io.stdout:write("\t") end
	bstr = string.format("0x%02x, ", string.byte(fstr, i))
	io.stdout:write(bstr)
	if col == 0 then io.stdout:write("\n") end
end
io.stdout:write('};\n\nstatic int '..name..'_buflen = '..#fstr..';\n\n')
