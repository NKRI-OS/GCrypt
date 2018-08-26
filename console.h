#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include "config.h"

class Console
{
public:
	// https://stackoverflow.com/questions/1413445/reading-a-password-from-stdcin#answer-1455007
	static void SetStdinEcho (bool enable = true);
	static void splitCommand (const std::string & str, std::vector <std::string> & arr);

	template <typename T>
	static void Begin (std::string label, T && commandReader);
};

void Console::splitCommand (const std::string & str, std::vector <std::string> & arr)
{
//	arr.clear ();
	arr.push_back (std::string ());
	for (char ch : str)
		if (ch == ' ')
			arr.push_back (std::string ());
		else arr.back ().push_back (ch);
}

void Console::SetStdinEcho(bool enable)
{
#ifdef WIN32
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);

	if( !enable )
		mode &= ~ENABLE_ECHO_INPUT;
	else
		mode |= ENABLE_ECHO_INPUT;

	SetConsoleMode(hStdin, mode );

#else
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	if( !enable )
		tty.c_lflag &= ~ECHO;
	else
		tty.c_lflag |= ECHO;

	(void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

template <typename T>
void Console::Begin (std::string label, T && commandReader)
{
	while (true)
	{
		std::cout << label;
		std::string read_command;
		std::getline (std::cin, read_command);
		std::vector <std::string> cmds;
		splitCommand (read_command, cmds);
		bool breakconsole = false;
		for (std::string & cmd : cmds)
			if (commandReader (cmd, label))
			{
				breakconsole = true;
				break;
			}
		if (breakconsole) break;
	}
}

#endif // CONSOLE_H
