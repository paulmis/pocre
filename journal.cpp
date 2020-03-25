#include "pch.h"
#include "journal.h"

std::string get_date(std::string format)
{
	time_t now = time(0);
	tm timeinfo;
	localtime_s(&timeinfo, &now);
	char buf[256];
	strftime(buf, 256, format.c_str(), &timeinfo);
	return buf;
}

std::string journal_path = "logs/" + get_date("%F-%H-%M-%d") + ".txt";
std::string log2str[5] = { "CRT", "ERR", "WRN", "INF", "INI" };

void journal(std::string message, log_type type, bool blank)
{
	static std::ofstream out(journal_path.c_str(), std::ofstream::app);
	static std::mutex mutex;
	mutex.lock();

	// Prepend the date to the message
	if (blank) message = std::string(28, ' ') + message + "\n";
	else	message = "[" + get_date() + "] [" + log2str[type] + "] " + message + "\n";

	// Write the message
	out << message;
	std::cout << message;

	mutex.unlock();
}