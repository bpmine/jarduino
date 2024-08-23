#include "Logger.h"

#include <arduino.h>

Logger logger;

void Logger::print(int val) const
{
  Serial.print(val);
}

void Logger::println(int val) const
{
  Serial.println(val);
}

void Logger::print(unsigned long val) const
{
  Serial.print(val);
}

void Logger::println(unsigned long val) const
{
  Serial.println(val);
}

void Logger::print(const char *val) const
{
  Serial.print(val);
}

void Logger::println(const char *val) const
{
  Serial.println(val);
}


