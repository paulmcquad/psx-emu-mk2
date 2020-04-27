#include <exception>

class mips_exception : public std::exception
{
};

class mips_overflow : public std::exception
{

};

class mips_sys_call : public std::exception
{

};

class mips_breakpoint : public std::exception
{

};

class mips_interrupt : public std::exception
{

};

class mips_bus_error_DBE : public std::exception
{

};