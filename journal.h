#pragma once

enum log_type
{
	log_critical, log_error, log_warn, log_info, log_init
};

void journal(std::string message, log_type type, bool blank = false);

inline void jinf(std::string message)
{
	journal(message, log_info);
}

inline void jwrn(std::string message)
{
	journal(message, log_warn);
}

inline void jerr(std::string message)
{
	journal(message, log_error);
}

inline void jcrt(std::string message)
{
	journal(message, log_critical);
}

inline void jini(std::string message)
{
	journal(message, log_init);
}

inline void jnull(std::string message)
{
	journal(message, log_info, true);
}