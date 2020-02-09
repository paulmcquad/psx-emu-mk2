#include <unordered_map>
#include <memory>

class Gpu;
class IOPorts
{
public:
	void init(std::shared_ptr<Gpu> _gpu);

	unsigned char* get(unsigned int address, bool read);

private:
	std::shared_ptr<Gpu> gpu = nullptr;
	// some of the memory registers return different values when read or written too
	unsigned char* read_access(unsigned int addr);
	unsigned char* write_access(unsigned int addr);
};