#include "gcrypt.h"

using namespace gcrypt;

class GCrypt::BytePair
{
public:
	ubyte a, b;
	BytePair (ubyte, ubyte);
};

GCrypt::BytePair::BytePair (ubyte a, ubyte b)
{
	this->a = a, this->b = b;
}

class GCrypt::BytePairs
{
	vector <ubyte> tmpA;
	vector <ubyte> tmpB;
	vector <ubyte> tmpC;
	vector <ulong> tmpS;
	ulong strsize, byteCount_helper;
	string keystring;
public:
	vector <GCrypt::BytePair> list;
	BytePairs (string);
	void checkAndPush (ubyte a, ubyte b, bool regtmp);
	void checkAndPushTmp (bool includeInverse);
	void clearTmp ();
	ulong byteCount (ubyte b);
};

void GCrypt::BytePairs::checkAndPush (ubyte a, ubyte b, bool regtmp)
{
	bool haveA = false, haveB = false;

	for (GCrypt::BytePair s : list)
	{
		if (s.a == a)
		{
			haveA = true;
			if (haveB) break;
		}
		if (s.b == b)
		{
			haveB = true;
			if (haveA) break;
		}
	}

	if (!haveA && !haveB)
		list.push_back (GCrypt::BytePair (a, b));
	else
	{
		if (regtmp)
		{
			if (!haveA) tmpA.push_back (a);
			if (!haveB) tmpB.push_back (b);
		}

		bool thA = false, thB = false;

		for (ulong i = 0; i < tmpC.size (); i++)
		{
			if (tmpC [i] == a)
			{
				thA = true;
				tmpS [i] ++;
				if (thB) break;
			}
			if (tmpC [i] == b)
			{
				thB = true;
				tmpS [i] ++;
				if (thA) break;
			}
		}

		if (!thA)
		{
			tmpC.push_back (a);
			tmpS.push_back (2);
		}
		if (!thB)
		{
			tmpC.push_back (b);
			tmpS.push_back (2);
		}
	}
}

void GCrypt::BytePairs::checkAndPushTmp (bool includeInverse)
{
	if (tmpA.size () == tmpB.size ())
		for (ulong i = 0; i < tmpA.size (); i++)
		{
			checkAndPush (tmpA [i], tmpB [i], false);
			if (includeInverse) checkAndPush (255 - tmpA [i], 255 - tmpB [i], false);
		}
}

void GCrypt::BytePairs::clearTmp ()
{
	tmpA.erase (tmpA.begin (), tmpA.end ());
	tmpB.erase (tmpB.begin (), tmpB.end ());
}

GCrypt::BytePairs::BytePairs (string str)
	: keystring (str), strsize (str.size ()), byteCount_helper (0)
{
	for (ulong i = 0; i < strsize; i++)
	{
		checkAndPush (str [i], str [i!=strsize-1?i+1:0], true);
		checkAndPush (255 - str [i], 255 - str [i!=strsize-1?i+1:0], true);
	}

	checkAndPushTmp (true);
	clearTmp ();

	vector <ubyte> fillA;
	vector <ubyte> fillB;

	for (ubyte b = 0; b < 256; b++)
	{
		bool haveA = false, haveB = false;
		for (BytePair s : list)
		{
			if (s.a == b)
			{
				haveA = true;
				if (haveB) break;
			}
			if (s.b == b)
			{
				haveB = true;
				if (haveA) break;
			}
		}

		if (!haveA) fillA.push_back (b);
		if (!haveB) fillB.push_back (b);

		if (b == 255) break;
	}

	ulong r = 0;

	for (ubyte b : str)
		r += b;
	r %= fillA.size ();

	rotate (fillA.begin (), fillA.begin () + r, fillA.end ());

	reverse (fillB.begin (), fillB.end ());

	r = 0;

	for (ulong i = 0; i < strsize; i++)
		if (i % 2)
			r += str [i];
		else
			r *= str [i];

	r %= fillA.size ();

	rotate (fillB.begin (), fillB.end () - r, fillB.end ());

	for (ubyte i = 0; i < fillA.size (); i++)
	{
		list.push_back (BytePair (fillA [i], fillB [i]));
		if (i == 255) break;
	}

	for (ulong & i : tmpS)
		i = ((i %= 256) ? i : (strsize % 256));
}

ulong GCrypt::BytePairs::byteCount (ubyte b)
{
	for (ulong i = 0; i < tmpC.size (); i++)
		if (b == tmpC [i])
			return tmpS [i];
	return ubyte (keystring [++byteCount_helper %= strsize]) % 256;
}

vector <ubyte> GCrypt::BytesTable ()
{
	vector <ubyte> table;
	for (ubyte b = 0; b < 256; b++)
	{
		table.push_back (b);
		if (b == 255) break;
	}
	return table;
}

void GCrypt::checkKey (GCrypt::BytePairs key)
{
	try
	{
		if (key.list.size () != 256)
			throw "Error! Could not create key.";

		vector <ubyte> checkA, checkB;
		for (ubyte i = 0; i < key.list.size (); i++)
		{
			checkA.push_back (key.list [i].a);
			checkB.push_back (key.list [i].a);
			if (i == 255) break;
		}

		for (ubyte i = 0; i < 256; i++)
		{
			for (ubyte j = 0; j < 256; j++)
			{
				if (checkA [0] == checkB [j])
				{
					checkA.erase (checkA.begin () + 0);
					checkB.erase (checkB.begin () + j);
					break;
				}
				if (j == 255) break;
			}
			if (i == 255) break;
		}

		if (checkA.size () || checkB.size ())
			throw "Error! Could not create key.";
	}
	catch (const char * message)
	{
		clog << message << endl;
	}
}

void GCrypt::encrypt (char * data, const ulong & datasize, const string & key)
{
	checkKey (key);
	GCrypt::BytePairs keyBytePairs (key);
	vector <ubyte> bytesTable = GCrypt::BytesTable ();

	for (ulong i = 0; i < datasize; i++)
	{
		for (BytePair s : keyBytePairs.list)
		{
			if (ubyte (data [i]) == s.a)
			{
				data [i] = bytesTable [s.b];
				rotate (bytesTable.begin (), bytesTable.begin () + keyBytePairs.byteCount (bytesTable [s.b]), bytesTable.end ());
				break;
			}
		}
	}
}

void GCrypt::decrypt (char * data, const ulong & datasize, const string & key)
{
//	checkKey (key); // The key is already checked when data is encrypted.
	GCrypt::BytePairs keyBytePairs (key);
	vector <ubyte> bytesTable = GCrypt::BytesTable ();

	for (ulong i = 0; i < datasize; i++)
	{
		for (BytePair s : keyBytePairs.list)
		{
			if (ubyte (data [i]) == bytesTable [s.b])
			{
				data [i] = s.a;
				rotate (bytesTable.begin (), bytesTable.begin () + keyBytePairs.byteCount (bytesTable [s.b]), bytesTable.end ());
				break;
			}
		}
	}
}
