#include <exception>

// reset
class reset : public std::exception
{
};

// IBE/DBE
class bus_error : public std::exception
{
};

// AdEL, AdES
class address_error : public std::exception
{
};

// Ovf
class overflow_exception : public std::exception
{
};

// Sys
class sys_call : public std::exception
{
};

// Bp
class breakpoint_exception : public std::exception
{
};

// RI
class reserved_instruction : public std::exception
{
};

// CpU
class coprocessor_unusable : public std::exception
{
};

// Int
class interrupt : public std::exception
{
};