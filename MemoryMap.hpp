#include <string>
#include <unordered_map>
#include <deque>

constexpr unsigned int MAIN_MEMORY_SIZE = 1024 * 512 * 4;
constexpr unsigned int PARALLEL_PORT_SIZE = 1024 * 64;
constexpr unsigned int SCRATCH_PAD_SIZE = 1024;
constexpr unsigned int IO_PORTS_SIZE = 1024 * 8;
constexpr unsigned int BIOS_SIZE = 1024 * 512;
constexpr unsigned int CACHE_CONTROL_SIZE = 512;

class IOPorts;

class Ram {
public:
	void init(std::string bios_filepath, std::shared_ptr<IOPorts> _io_ports);
	void reset();

	// =============================================================================
	// These load/store templates replace a series of somewhat nasty reinterpret_casts
	// of unsigned char pointers, I wasn't happy with relying on potentially unaligned
	// and heavily aliased accesses so felt these templates were acceptable replacements.
	//
	// For the moment i'm not going to bother checking for unaligned loads or stores as
	// I am going to work under the assumption that normal programs don't do them
	// except through the specific unaligned loading/storing instructions
	// =============================================================================
	template <class T>
	T load(unsigned int address)
	{
		T result = 0;

		int num_bytes = sizeof(T);
		for (int offset = num_bytes - 1; offset >= 0; offset--)
		{
			unsigned int current_address = address + offset;
			result <<= 8;
			result |= load<unsigned char>(current_address);
		}

		return result;
	};

	template <>
	unsigned char load<unsigned char>(unsigned int address)
	{
		unsigned char *memory = get_memory_ptr(address, true);
		return *memory;
	};

	template <class T>
	void store(unsigned int address, T value)
	{
		int num_bytes = sizeof(T);
		for (int offset = 0; offset < num_bytes; offset++)
		{
			unsigned int current_address = address + offset;
			unsigned char byte_value = value & 0xFF;
			value >>= 8;
			store<unsigned char>(current_address, byte_value);
		}
	};

	template<>
	void store<unsigned char>(unsigned int address, unsigned char value)
	{
		unsigned char *memory = get_memory_ptr(address, true);
		*memory = value;
	};

private:

	unsigned char* get_memory_ptr(unsigned int address, bool read_access);

	// four SRAM chips of 512KB
	unsigned char memory[MAIN_MEMORY_SIZE] = { 0 };
	unsigned char parallel_port[PARALLEL_PORT_SIZE] = { 0 };
	unsigned char scratch_pad[SCRATCH_PAD_SIZE] = { 0 };
	unsigned char bios[BIOS_SIZE] = { 0 };
	unsigned char cache_control[CACHE_CONTROL_SIZE] = { 0 };
	std::shared_ptr<IOPorts> io_ports;
};