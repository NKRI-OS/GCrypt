#ifndef GCRYPT_H
#define GCRYPT_H

typedef unsigned char ubyte;

void gHash (const ubyte * const key, unsigned long long key_size, ubyte hash [256])
{
	ubyte h1 [256];
	unsigned long long i, j;
	ubyte s, sm, b, bi, si, wi;
	for (i = 0; i < 256; i++) h1 [i] = (ubyte) i;
	for (i = 0; i < key_size; i++)
	{
		b = key [i];
		if (b == 0) continue;
		s = 256 / b;
		sm = 256 % b;
		if (sm != 0) s++;
		wi = b;
		for (bi = 0; bi < b; bi++)
		{
			if (bi == sm) s--;
			for (si = 0; si < s; si++)
				hash [wi++] = h1 [(ubyte) (b * si + bi)];
		}
		for (j = 0; j < 256; j++) h1 [j] = hash [j];
	}
}

void gEncrypt (const ubyte * const data, unsigned long long data_size, const ubyte * const key, unsigned long long key_size, ubyte * output)
{
	ubyte hash [256];
	ubyte rot = 0;
	gHash (key, key_size, hash);

	for (unsigned long long i = 0; i < data_size; i++)
		output [i] = hash [rot += data [i]];
}

void gDecrypt (const ubyte * const data, unsigned long long data_size, const ubyte * const key, unsigned long long key_size, ubyte * output)
{
	ubyte hash [256];
	ubyte rot = 0;
	gHash (key, key_size, hash);

	for (unsigned long long i = 0; i < data_size; i++)
	{
		ubyte b = 0;
		for (; hash [b] != data [i]; b++);
		output [i] = (ubyte) (b - rot);
		rot += data [i];
	}
}

#endif // GCRYPT_H
