constexpr unsigned int SPU_CONTROL_SIZE = 64;
constexpr unsigned int SPU_VOICE_SIZE = 576;

class Spu
{
public:
	void init();
	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);
private:
	unsigned char spu_control[SPU_CONTROL_SIZE] = { 0 };
	unsigned char spu_voice_registers[SPU_VOICE_SIZE] = { 0 };
};