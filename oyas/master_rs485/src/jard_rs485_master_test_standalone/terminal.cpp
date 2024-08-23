/**
 * @file terminal.cpp
 * @brief Gestion du terminal
 *
 * @todo Ajouter les comandes setdate, settime
 **/
#ifndef TERMINAL_HEADER_INCLUDED
#define TERMINAL_HEADER_INCLUDED

#include <arduino.h>

char _bufferTerm[200];
int _posBufferTerm=0;

static void _help(void)
{
  Serial.println("____________________");
  Serial.println("Aide:");
}

static void _execCmd(const char *strCmd,const char *strParams)
{
  if (strcmp(strCmd,"test")==0)
  {
    Serial.println("TEST OK");
  }
  else if (strcmp(strCmd,"help")==0)
  {
    _help();
  }
}

void serialEvent(void)
{
  if (Serial.available()>0)
  {
    String st=Serial.readString();
    if (st.length()>20)
      return;

    st.trim();

    int inx=st.indexOf(" ");
    if (inx==-1)
    {
      _execCmd(st.c_str(),NULL);
    }
    else
    {
      _execCmd(st.substring(0,inx).c_str(),st.substring(inx+1).c_str());
    }
  }
}


#endif
