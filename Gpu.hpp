class Gpu
{
public:
	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GP0_send;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GP1_send;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GPUREAD;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GPUSTAT;
};