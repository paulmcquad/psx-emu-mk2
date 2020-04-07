#include <memory>
#include <map>
#include <unordered_map>
#include "InstructionTypes.hpp"

class Cop0;
class Cop2;
class Ram;

enum class cpu_instructions : unsigned char;
enum class cpu_special_funcs : unsigned char;
enum class cpu_bconds : unsigned char;

// ref:
// https://svkt.org/~simias/guide.pdf
// https://problemkaputt.de/psx-spx.htm
// http://hitmen.c02.at/files/docs/psx/psx.pdf
class Cpu : public std::enable_shared_from_this<Cpu>
{
public:
	std::shared_ptr<Cop0> cop0 = nullptr;
	std::shared_ptr<Cop2> cop2 = nullptr;

	std::shared_ptr<Ram> ram = nullptr;

	// 3 stage register file to simulate the load delay
	// shadow first -> shadow second -> gp_registers
	struct
	{
		unsigned int gp_registers[32] = { 0 };
		unsigned int shadow_gp_registers_last[32] = { 0 };
		unsigned int shadow_gp_registers_first[32] = { 0 };
		void merge()
		{
			memcpy(&gp_registers, &shadow_gp_registers_last, sizeof(unsigned int) * 32);
			memcpy(&shadow_gp_registers_last, &shadow_gp_registers_first, sizeof(unsigned int) * 32);
		}
		void reset()
		{
			for (int index = 0; index < 32; index++)
			{
				gp_registers[index] = shadow_gp_registers_first[index] = shadow_gp_registers_last[index] = 0;
			}
		}
	} register_file;

	void init(std::shared_ptr<Ram> _ram);
	void reset();
	void tick();

	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	void execute(unsigned int instruction);
	void execute_special(const instruction_union& instr);
	void execute_bcond(const instruction_union& instr);
	void execute_cop(const instruction_union& instr);

	unsigned int get_register(int index);
	void set_register(int index, unsigned int value, bool delay = false);

	unsigned int get_immediate_base_addr(const instruction_union& instr);

	unsigned int hi = 0;
	unsigned int lo = 0;

	unsigned int current_pc = 0;
	unsigned int next_pc = 0;

	unsigned int current_instruction = 0;
	unsigned int next_instruction = 0;

	bool in_delay_slot = false;
};