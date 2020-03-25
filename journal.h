#pragma once

// Continues
#define NO_ARGS_C				   {jerr("Expected an argument but no arguments have been provided"); continue;}
#define INVALID_ARG_C			   {jerr("Some arguments are invalid"); continue;}
#define CANNOT_OPEN_FILE_C(path)   {jerr("Couldn't open file at " + path); continue;}
#define MISSING_DATA_C(path)	   {jerr("Missing data from file " + path); continue;} 
#define MISSING_DIRECTORY_C(path)  {jerr("Missing " + path + " directory"); continue;}
#define EMPTY_IMAGE_C			   {jerr("Image is empty"); continue;}
#define ITEM_NOT_FOUND_C(id)       {jerr("Item " + std::to_string(id) + " hasn't been found"); continue;}
#define EX_QUERY_FAIL_C(item)	   {jerr("Exchange query of item " + item + " failed"); continue;}
#define INVALID_MOD_C(mod)		   {jerr("Mod " + mod + " is invalid"); continue;}

// Falses
#define MISSING_FILE_DATA_F(path)  {jerr("Missing data from " + path); return false;} 
#define MISSING_DIRECTORY_F(path)  {jerr("Missing " + path + " directory"); return false;}
#define MISSING_FILE_F(path)       {jerr("Missing " + path + " file"); return false;}
#define CANNOT_CREATE_FILE_F(path) {jerr("Cannot create file at " + path); return false;}
#define CANNOT_OPEN_FILE_F(path)   {jerr("Couldn't open file at " + path); return false;}
#define ITEM_NOT_FOUND_F(id)	   {jerr("Item " + std::to_string(id) + " hasn't been found"); return false;}
#define QUERY_FAILED(id)		   {jerr("Querying item " + std::to_string(id) + " failed"); return false;}

std::string get_date(std::string format = "%F %X");

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