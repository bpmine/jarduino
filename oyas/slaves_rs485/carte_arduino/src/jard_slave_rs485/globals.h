/**
 * @file globals.h
 * 
 * Variables globales (partagees entre gestion esclave et main)
 */
#ifndef GLOBALS_INCLUDED
#define GLOBALS_INCLUDED

extern bool g_on;
extern bool g_cpt_low;
extern bool g_cpt_high;

extern unsigned char g_mes_cv;
extern int g_flow_mLpMin;
extern char g_temp_dg;
extern char g_hum_pc;

extern unsigned short g_total_s;
extern unsigned short g_errors;

extern void reset_time(void);
extern void reset_errs(void);

#endif
