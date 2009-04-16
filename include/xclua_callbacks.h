#ifndef XCLUA_CALLBACK_H
#define XCLUA_CALLBACK_H

int xclua_callback(char ** word, void * ud);
int xclua_callback_command(char ** word, char ** word_eol, void * ud);
int xclua_callback_server(char ** word, char ** word_eol, void * ud);
int xclua_callback_timer(void * ud);
#define xclua_callback_print	xclua_callback
// xclua_callback_fd -- not supported

#endif
