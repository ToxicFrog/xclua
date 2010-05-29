.. :wrap=soft: ..

=========
xchat-lua
=========

.. contents ::

Introduction
============

xchat-lua is an XChat plugin, written in C, that allows one to develop and use xchat plugins using the Lua programming language.

This document assumes basic familiarity with the Lua programming language.


Basic features
==============

Current
-------
* Loading and unloading of lua scripts
* Support for most of the XChat API (excepting raw file descriptors)
* Per plugin, wholly independent interpreter states

Planned
-------
* Automatic loading of Lua scripts
* /lua command
* Interactive Lua console in xchat
* Automatic IO redirection

Module Development
==================

Basics
------

FIXME

Startup and Shutdown
--------------------

FIXME

``xchat.init(name)``
~~~~~~~~~~~~~~~~~~~~

FIXME

``xchat.deinit()``
~~~~~~~~~~~~~~~~~~

FIXME

Communication with the Outside World
------------------------------------

``xchat.print(str)``
~~~~~~~~~~~~~~~~~~~~
Outputs the given string to the current tab.

``xchat.printf(fmt, ...)``
~~~~~~~~~~~~~~~~~~~~~~~~~~
Outputs formatted text to the current tab. Equivalent to ``xchat.print(string.format(fmt, ...))``.

``xchat.command(str)``
~~~~~~~~~~~~~~~~~~~~~~
Executes the given string as a command typed into the xchat input box. A leading / is not needed.

``xchat.commandf(fmt, ...)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Executes formatted text. Equivalent to using ``xchat.command(string.format(fmt, ...))``.

``xchat.printe(event, ...)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Generates a print event. The string ``event`` can be any event listed in the Settings->Advanced->Text Events window in xchat. The remaining arguments are the arguments to the event; for example, for a "Change Nick" event, it would be called as ``xchat.printe("Change Nick", "OldNick", "NewNick")``.

The use of this function can trigger print event callbacks (created with ``xchat.hook_print``). Be careful not to cause an infinite loop.

``xchat.send_modes(mode, ...)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets or unsets the given mode, in the current channel, on each nick listed. For example, the following command will give the three people listed ops:

    ``xchat.send_modes("+o", "Echo", "Victor", "Sierra")``

``xchat.send_modes(mode, targets)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Equivalent to ``xchat.send_modes(mode, unpack(targets))``.

Querying XChat
--------------

``xchat.list(names...)``
~~~~~~~~~~~~~~~~~~~~~~~~
For each name, returns a list of information for the current context.

    FIXME: this needs to be documented, there's a lot of tables.

``xchat.pref(names...)``
~~~~~~~~~~~~~~~~~~~~~~~~
For each name, returns the corresponding xchat preference. You can get a list of them with ``/set``.

Two names are available that are not listed in ``/set``. ``"state_cursor"`` returns the current position, in characters, of the text entry cursor in the text box. ``"id"`` returns an ID unique to the current server.

``xchat.info(names...)``
~~~~~~~~~~~~~~~~~~~~~~~~
For each name, returns a piece of information based on the current context. Currently supported names (as of xchat 2.8.2) are:

    FIXME - need a table here

``xchat.nickcmp(nick1, nick2)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compares the two nicks given using the nick comparison rules of the current IRC server. Of particular interest is that on RFC1459-compliant servers, the characters '{}|' are considered the lower-case versions of '[]\\'.

It returns a number less than, equal to, or greater than zero if ``nick1`` is found to be less than, equal to, or greater than ``nick2``.

``xchat.strip(str)``
~~~~~~~~~~~~~~~~~~~~
Returns a copy of ``str`` with all colour and formatting codes removed.


Context Management
------------------

``xchat.get_context()``
~~~~~~~~~~~~~~~~~~~~~~~
Returns a userdatum representing the current xchat context. This can be passed later to ``xchat.set_context``.

``xchat.find_context(server, channel)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns a context for the given channel and server. If ``server`` is **nil**, finds any channel (or other tab) by the given name. If ``channel`` is **nil**, returns the frontmost tab in the given server. If both are **nil**, it behaves equivalently to ``xchat.get_context``.

If no context matching the given names can be found, returns **nil**.

``xchat.set_context(ctx)``
~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the current xchat context. ``ctx`` must be a userdatum previously acquired using ``xchat.get_context`` or ``xchat.find_context``.


Hooks and Callbacks
-------------------

FIXME - overview of hooks and callbacks goes here

Priorities
~~~~~~~~~~
``command``, ``server``, and ``print`` hooks must be assigned a priority when created. This must be one of ``xchat.PRI_HIGHEST``, ``xchat.PRI_HIGH``, ``xchat.PRI_NORMAL``, ``xchat.PRI_LOW``, or ``xchat.PRI_LOWEST``.

The recommended value - and default if omitted - is ``xchat.PRI_NORMAL``. The effect of other values is undefined.

Return Values
~~~~~~~~~~~~~
Each hook function, when called, must return a value indicating what other code (if any) is allowed to see the event that triggered it. The possible values are:

    =================== =======
    Value               Meaning
    =================== =======
    xchat.EAT_NONE      Permit both other hooks and xchat itself to process this event.
    xchat.EAT_XCHAT     Permit other hooks to process this event, but not xchat.
    xchat.EAT_PLUGIN    Permit xchat to process this event, but not other hooks.
    xchat.EAT_ALL       Hide this event both from xchat and from other hooks.
    =================== =======

``xchat.hook_command(command, fn, priority, help)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hooks a function to be called when a given command is entered with a leading '/'. To capture all text entered, use "" as the command. Commands starting with '.' will not appear in the output of ``/help``.

``priority`` is optional and defaults to ``xchat.PRI_NORMAL``.
``help`` is also optional. If present it will be the text displayed for ``/help command``.

``xchat.hook_server(message, fn, priority)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hooks a function to be called when a given message arrives from the server. ``message`` can be any raw message ("NICK", "PRIVMSG", etc); a server numeric in string form (such as "006"); or the special form "RAW LINE" to capture every line received by the server.

``priority`` is optional and defaults to ``xchat.PRI_NORMAL``.

``xchat.hook_print(event, fn, priority)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hooks a function to be called when a given text event is printed. In addition to the events available in the "Text Events" window in xchat, the following special forms are available:

=============== ===========
Event Name      Called When
=============== ===========
"Close Context" An xchat context (ie, a tab) is closed.
"Focus Tab"     A tab is focused.
"Focus Window"  A top-level window is focused.
"DCC Chat Text" Text arrives in a DCC chat session. The callback is passed the address, port, and nick of the sender as the first three elements in ``word``.
"Key Press"     A key is pressed in the input box. ``word`` contains the key scancode, modifier key mask, key string, and key string length. Of these ``word[3]`` is probably most useful. It may be **nil** if the key is nonprintable.
=============== ===========

``xchat.hook_timer(timeout, fn)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hooks a function to be called every ``timeout`` milliseconds. It will not automatically stop, but must be explicitly disabled when done with using ``xchat.unhook``.

``xchat.unhook(hook)``
~~~~~~~~~~~~~~~~~~~~~~
Releases a hook previously returned by ``xchat.hook_*``.

