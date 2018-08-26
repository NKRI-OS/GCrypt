#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include "gcrypt.h"
#include "console.h"

using namespace std;
using namespace gcrypt;

void processAndWrite (void (* process_data) (char * data, const ulong & datasize, const string & key), string key, string input_file, string output_file);

#ifdef __linux__

char header [] =
	"\033[1;31m"
	" _______    _____    _______  _      _  _______   _______   /\\ \n"
	"|  ___  |  /_____\\  | _____ \\ \\\\    // | _____ \\\ |___ ___| /||\\\n"
	"| |   |_| //     \\\\ ||     ||  \\\\  //  ||     ||    | |     || \n"
	"| |  ___  ||        ||_____||   \\\\//   ||_____||    | |     || \n"
	"| | |_  | ||        ||_____ /    \\/    ||______/    | |     || \n"
	"| |   | | ||      _ ||     \\\\\    ||    ||           | |     || \n"
	"| |___| | \\\\_____// ||     ||    ||    ||           | |     || \n"
	"|_______|  \\_____/  ||     ||    ||    ||           |_|     || \n"
	"\033[0m"
	"                                                           \033[1;31m || \033[0m\n"
	"\033[1;33mGCrypt v0.1 - https://github.com/GasparVardanyan/GCrypt\033[0m    \033[1;31m || \033[0m\n"
	"                                                           \033[1;31m || \033[0m\n"
	"Options:                                                   \033[1;31m || \033[0m\n"
	"\t--help\t\t\tDisplay this help message  \033[1;31m || \033[0m\n"
	"\t-e\t\t\tEncryption mode            \033[1;31m || \033[0m\n"
	"\t-d\t\t\tDecryption mode            \033[1;31m || \033[0m\n"
	"\t-c\t\t\tConsole mode               \033[1;31m || \033[0m\n"
	"Console commands:                                          \033[1;31m || \033[0m\n"
	"\tencrypt\t\t\tEncrypt file               \033[1;31m || \033[0m\n"
	"\tdecrypt\t\t\tDecrypt file               \033[1;31m || \033[0m\n"
	"\texit\t\t\tExit console               \033[1;31m || \033[0m\n"
	"Usage:                                                     \033[1;31m || \033[0m\n"
	"\tgcrypt [mode] [[key] [inputfile] [outputfile]]     \033[1;31m || \033[0m\n"
	"Examples:                                                  \033[1;31m || \033[0m\n"
	"\tgcrypt -e samplekey file_to_encrypt outputfile     \033[1;31m || \033[0m\n"
	"\tgcrypt -d samplekey file_to_decrypt outputfile     \033[1;31m || \033[0m\n"
	"\033[1;31m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\033[0m\n"
;

string cmd_label = "\033[1;31mgcrypt\033[0m> ";

#else // __linux__

char header [] =
	""
	" _______    _____    _______  _      _  _______   _______   /\\ \n"
	"|  ___  |  /_____\\  | _____ \\ \\\\    // | _____ \\\ |___ ___| /||\\\n"
	"| |   |_| //     \\\\ ||     ||  \\\\  //  ||     ||    | |     || \n"
	"| |  ___  ||        ||_____||   \\\\//   ||_____||    | |     || \n"
	"| | |_  | ||        ||_____ /    \\/    ||______/    | |     || \n"
	"| |   | | ||      _ ||     \\\\\    ||    ||           | |     || \n"
	"| |___| | \\\\_____// ||     ||    ||    ||           | |     || \n"
	"|_______|  \\_____/  ||     ||    ||    ||           |_|     || \n"
	""
	"                                                            || \n"
	"GCrypt v0.1 - https://github.com/GasparVardanyan/GCrypt     || \n"
	"                                                            || \n"
	"Options:                                                    || \n"
	"\t--help\t\t\tDisplay this help message   || \n"
	"\t-e\t\t\tEncryption mode             || \n"
	"\t-d\t\t\tDecryption mode             || \n"
	"\t-c\t\t\tConsole mode                || \n"
	"Console commands:                                           || \n"
	"\tencrypt\t\t\tEncrypt file                || \n"
	"\tdecrypt\t\t\tDecrypt file                || \n"
	"\texit\t\t\tExit console                || \n"
	"Usage:                                                      || \n"
	"\tgcrypt [mode] [[key] [inputfile] [outputfile]]      || \n"
	"Examples:                                                   || \n"
	"\tgcrypt -e samplekey file_to_encrypt outputfile      || \n"
	"\tgcrypt -d samplekey file_to_decrypt outputfile      || \n"
	"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
;

string cmd_label = "gcrypt> ";

#endif // __linux__

char argumentsError [] =
	"Error: invalid arguments detected.\n"
	"Usage: gcrypt [mode] [[key] [inputfile] [outputfile]]\n"
;

char inputFileNotFound [] =
	"Error: input file not found.\n"
	"Usage: gcrypt [mode] [[key] [inputfile] [outputfile]]\n"
;

char outputFileDirNotFound [] =
	"Error: output file directory not found.\n"
	"Usage: gcrypt [mode] [[key] [inputfile] [outputfile]]\n"
;

#define CMDEXEC_START "CMDEXEC_START"
#define CMDEXEC_CRYPT_I "CMDEXEC_CRYPT_I"
#define CMDEXEC_CRYPT_O "CMDEXEC_CRYPT_O"
#define CMDEXEC_CRYPT_K "CMDEXEC_CRYPT_K"
#define CMDEXEC_CRYPT_R "CMDEXEC_CRYPT_R"
#define CMD_ENCRYPT "encrypt"
#define CMD_DECRYPT "decrypt"
#define CMD_EXIT "exit"

int main (int argc, char * argv [])
{
	void (* process_data) (char * data, const ulong & datasize, const string & key);
	string key, input_file, output_file;
	bool process = false;

	if (argc == 1)
		cout << header;
	else if (argc == 2)
		if (strcmp (argv [1], "--help") == 0)
			cout << header;
		else if (strcmp (argv [1], "-c") == 0)
			Console::Begin (cmd_label, [&] (string & command, string & label)
			{
				static string cmdexec = CMDEXEC_START;
				static bool needConfirmKey;
				if (cmdexec == CMDEXEC_START)
				{
					if (command == CMD_ENCRYPT)
					{
						cmdexec = CMDEXEC_CRYPT_I;
						label = "Input file:\t";
						process_data = & GCrypt::encrypt;
						needConfirmKey = true;
					}
					else if (command == CMD_DECRYPT)
					{
						cmdexec = CMDEXEC_CRYPT_I;
						label = "Input file:\t";
						process_data = & GCrypt::decrypt;
						needConfirmKey = false;
					}
					else if (command == "exit")
					{
						cmdexec = "start";
						return 1;
					}
					else if (command != "")
					{
						cout << "Error: unknown command \"" << command << "\"." << endl;
					}
				}
				else if (cmdexec == CMDEXEC_CRYPT_I)
				{
					input_file = command;
					cmdexec = CMDEXEC_CRYPT_O;
					label = "Output file:\t";
				}
				else if (cmdexec == CMDEXEC_CRYPT_O)
				{
					output_file = command;
					cmdexec = CMDEXEC_CRYPT_K;
					label = "Key:\t\t";
					Console::SetStdinEcho (false);
				}
				else if (cmdexec == CMDEXEC_CRYPT_K)
				{
					cout << endl;
					if (command != "")
					{
						key = command;
						if (needConfirmKey)
						{
							cmdexec = CMDEXEC_CRYPT_R;
							label = "Confirm key:\t";
						}
						else
						{
							cmdexec = CMDEXEC_START;
							label = cmd_label;
							Console::SetStdinEcho (true);
							processAndWrite (process_data, key, input_file, output_file);
							input_file = output_file = key = "";
						}
					}
					else cout << "Error: key is empty." << endl;
				}
				else if (cmdexec == CMDEXEC_CRYPT_R)
				{
					cout << endl;
					if (command == key)
					{
						cmdexec = CMDEXEC_START;
						label = cmd_label;
						Console::SetStdinEcho (true);
						processAndWrite (process_data, key, input_file, output_file);
						input_file = output_file = key = "";
					}
					else
					{
						cout << "Error: key does not match the confirm key." << endl;
						cmdexec = CMDEXEC_CRYPT_K;
						label = "Key:\t\t";
					}
				}

				return 0;
			});
		else
		{
			cerr << argumentsError;
			return 1;
		}
	else if (argc == 5)
	{
		if (strcmp (argv [1], "-e") == 0) process_data = & GCrypt::encrypt;
		else if (strcmp (argv [1], "-d") == 0) process_data = & GCrypt::decrypt;
		else
		{
			cerr << argumentsError;
			return 1;
		}
		key = argv [2], input_file = argv [3], output_file = argv [4];
		process = true;
	}
	else
	{
		cerr << argumentsError;
		return 1;
	}

	if (process) processAndWrite (process_data, key, input_file, output_file);
	input_file = output_file = key = "";

	return 0;
}

void processAndWrite (void (* process_data) (char * data, const ulong & datasize, const string & key), string key, string input_file, string output_file)
{
	ifstream file_read (input_file, ios_base::in | ios_base::binary);
	if (file_read.fail ())
	{
		cerr << inputFileNotFound;
		return;
	}

	ulong pos, start, end, filesize;

	pos = file_read.tellg ();
	file_read.seekg (0, ios_base::beg);
	start = file_read.tellg ();
	file_read.seekg (0, ios_base::end);
	end = file_read.tellg ();
	file_read.seekg (pos, ios_base::beg);
	filesize = end - start;

	char * bytearray = new char [filesize];
	file_read.read (bytearray, filesize);

	file_read.close ();

	ofstream file_write (output_file, ios_base::out | ios_base::trunc | ios_base::binary);
	if (file_write.fail ())
	{
		cerr << outputFileDirNotFound;
		return;
	}
	process_data (bytearray, filesize, key);
	file_write.write (bytearray, filesize);

	cout << '\a';

	delete [] bytearray;
	file_write.close ();
}
