#pragma once

constexpr int MaxItemlist = 6500;

struct TOD_ItemList
{
	char Name[64];

	short Mesh;
	short SubMesh;

	short unk;

	short Level;
	short STR;
	short INT;
	short DEX;
	short CON;

	struct
	{
		short Index;
		short Value;
	} Effect[12];

	int Price;
	short Unique;
	short Pos;

	short Extreme;
	short Grade;
};