#ifndef GCRYPT_H
#define GCRYPT_H

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include "config.h"

using namespace std;

namespace gcrypt
{
	typedef unsigned char ubyte;

	class GCrypt
	{
	private:
		class BytePair;
		class BytePairs;
		static vector <ubyte> BytesTable ();
		static void checkKey (BytePairs key);
	public:
		static void encrypt (char * data, const ulong & datasize, const string & key);
		static void decrypt (char * data, const ulong & datasize, const string & key);
	};
}

#endif // GCRYPT_H
