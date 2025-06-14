/**
 * @file globals.h
 *
 * @brief Definitions globales / configuration en dur
**/
#ifndef GLOBALS_HEADER_INCLUDED
#define GLOBALS_HEADER_INCLUDED

#define NODE_BARBEC
//#define NODE_REDUIT
//#define NODE_PAUL

#if defined(NODE_BARBEC) && !defined(NODE_REDUIT) && !defined(NODE_PAUL)
  #warning Noeud BARBEC
#elif !defined(NODE_BARBEC) && defined(NODE_REDUIT) && !defined(NODE_PAUL)
  #warning Noeud REDUIT
#elif !defined(NODE_BARBEC) && !defined(NODE_REDUIT) && defined(NODE_PAUL)
  #warning Noeud PAUL
#else
  #error Aucun noeud defini
#endif

#endif
