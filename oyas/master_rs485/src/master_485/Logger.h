#ifndef LOGGER_HEADER_INCLUDED
#define LOGGER_HEADER_INCLUDED

constexpr int LOG_STATES = 1;

class Logger
{
  private:
    int channel;

  public:
    Logger(int ch = 0) : channel(ch) {}
    virtual ~Logger() {}

    void print(int value) const;
    void println(int value) const;
    void print(unsigned long value) const;
    void println(unsigned long value) const;
    void print(const char *message) const;
    void println(const char *message) const;
};

extern Logger logger;

#endif
