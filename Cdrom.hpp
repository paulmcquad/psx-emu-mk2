#include <string>
#include <vector>

class Cdrom
{
public:
	void init();
	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	bool load(std::string bin_path, std::string cue_path);
private:
	unsigned int num_sectors = 0;
	std::vector<unsigned char> rom_data;
};