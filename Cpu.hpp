#include <memory>
#include <map>
#include <unordered_map>
#include "InstructionTypes.hpp"
#include "RegisterFile.hpp"

class SystemControlCoprocessor;
class GTECoprocessor;
class Bus;

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
	std::shared_ptr<SystemControlCoprocessor> cop0 = nullptr;
	std::shared_ptr<GTECoprocessor> cop2 = nullptr;
	std::shared_ptr<Bus> bus = nullptr;

	RegisterFile register_file;

	void init(std::shared_ptr<Bus> _bus);
	void reset();
	void tick();

	void execute_mips_exception(unsigned int excode);

	void save_state(std::stringstream& file);
	void load_state(std::stringstream& file);

	void execute(const instruction_union& instruction);
	void execute_special(const instruction_union& instr);
	void execute_bcond(const instruction_union& instr);
	void execute_cop(const instruction_union& instr);

	unsigned int get_immediate_base_addr(const instruction_union& instr);

	unsigned int hi = 0;
	unsigned int lo = 0;

	unsigned int current_pc = 0;
	unsigned int next_pc = 0;

	unsigned int current_instruction = 0;
	unsigned int next_instruction = 0;

	bool in_delay_slot = false;

	bool pending_exception = false;
	unsigned int pending_exception_excode = 0;
};