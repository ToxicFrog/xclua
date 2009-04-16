local _XCLUA_VERSION = "1.0 RC1"

function xchat.printf(format, ...)
	return xchat.print(string.format(format, ...))
end

function xchat.commandf(format, ...)
	return xchat.command(string.format(format, ...))
end
