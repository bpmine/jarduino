#ifndef GLOBALS_HEADER_INCLUDED
#define GLOBALS_HEADER_INCLUDED

#define NODE_BARBEC
//#define NODE_REDUIT

#if defined(NODE_BARBEC) && !defined(NODE_REDUIT)
  #warning Noeud BARBEC
#elif !defined(NODE_BARBEC) && defined(NODE_REDUIT)
  #warning Noeud REDUIT
#else
  #error Aucun noeud defini
#endif

#endif
