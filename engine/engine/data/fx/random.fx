//Variation Robert Jenkins' 96 bit Mix Function (original uses on more column of substractions, a isn't very random)
// a has the worst random quality of the three
uint3 bobJenkins_96(uint a, uint b, uint c)
{
	a=a-c;  a=a^(c >> 13);
	b=b-a;  b=b^(a << 8); 
	c=c-b;  c=c^(b >> 13);
	a=a-c;  a=a^(c >> 12);
	b=b-a;  b=b^(a << 16);
	c=c-b;  c=c^(b >> 5);
	a=a-c;  a=a^(c >> 3);
	b=b-a;  b=b^(a << 10);
	c=c-b;  c=c^(b >> 15);
	return uint3(c,b,a);
}

uint3 hash(float2 uv)
{
	return bobJenkins_96(asuint(uv.x), asuint(uv.y), 0x186FA32E);
}


float4 extractUnorm_8(uint h)
{
	return float4(
		((h&0xFF000000)>>24)/255.f,
		((h&0x00FF0000)>>16)/255.f,
		((h&0x0000FF00)>> 8)/255.f,
		((h&0x000000FF)    )/255.f);
}

float2 extractUnorm_16(uint h)
{
	return float2(
		((h&0xFFFF0000)>>16)/65535.f,
		((h&0x0000FFFF)    )/65535.f);
}

float extractUnorm_32(uint h)
{
	return h/4294967295.f;
}
