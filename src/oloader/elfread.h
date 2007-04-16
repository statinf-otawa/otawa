/**************************************************************************** 
                                  elfread.h                                   
                              ------------------                              
    date            : May 2003                                               
    copyright       : Institut de Recherche en Informatique de Toulouse       
    author          : Marc Finet                                              
    email           : finet@irit.fr, sainrat@irit.fr                        
 ****************************************************************************/
                                                                              
/**************************************************************************** 
 *                                                                          * 
 *   This program is free software; you can redistribute it and/or modify   * 
 *   it under the terms of the GNU General Public License as published by   * 
 *   the Free Software Foundation; either version 2 of the License, or      * 
 *   (at your option) any later version.                                    * 
 *                                                                          * 
 ****************************************************************************/

#ifndef _ELFREAD_H
#define _ELFREAD_H 1

#include "elf.h"

typedef unsigned short uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef struct tables {
	int32_t sechdr_tbl_size;
	Elf32_Shdr *sec_header_tbl;
	int32_t secnmtbl_ndx;
	char *sec_name_tbl;
        
	int32_t symtbl_ndx;
	Elf32_Sym *sym_tbl;
	char *symstr_tbl;

	int32_t dysymtbl_ndx;
	Elf32_Sym *dysym_tbl;
	char *dystr_tbl;

	int32_t hashtbl_ndx;
	Elf32_Word *hash_tbl;
	Elf32_Sym *hashsym_tbl;

        int32_t     pgm_hdr_tbl_size;
        Elf32_Phdr *pgm_header_tbl;
} Elf_Tables;

struct text_secs{
	char name[20];
	uint32_t offset;
	uint32_t address;
	uint32_t size;
        uint8_t *bytes;
	struct text_secs *next;
};

struct text_info{
	uint16_t txt_index;
	uint32_t address;
	uint32_t size;
	uint32_t txt_addr;
	uint32_t txt_size;
	uint8_t *bytes;
	struct text_secs *secs;
};

struct data_secs{
	char name[20];
	uint32_t offset;
	uint32_t address;
	uint32_t size;
	uint32_t type;
	uint32_t flags;
        uint8_t *bytes;
	struct data_secs *next;
};

struct data_info{
	uint32_t address;
	uint32_t size;
	struct data_secs *secs;
};

extern int Is_Elf_Little;
extern char Elf_Error;
extern Elf_Tables Tables;
extern struct text_info Text;
extern struct data_info Data;
extern uint32_t Stack_Addr;


int16_t   ElfReadInstrBytes ( unsigned char *, int16_t );
int16_t   ElfReadInstrBytesFrom ( unsigned char *, uint32_t, int16_t );

void      ElfRead ( int  );

uint32_t  GetMainAddr ( void );
uint8_t  *GetCodeBase ( void );                               
uint32_t  GetCodeBaseAddr ( void );                          
uint32_t  GetCodeSize ( void );                             
uint8_t  *GetDataBase ( void );                            
uint32_t  GetDataBaseAddr ( void );                       

uint32_t  GetDataAddr ( void );
uint32_t  GetDataSize ( void );                          



#if 0
int   ElfReadHeader ( int );
int   ElfReadSecNameTbl ( int);
int   ElfReadSymTbl ( int  );
int   ElfReadDySymTbl ( int  );
int   ElfReadHashTbl ( int  );
int   ElfReadTextSecs ( int  );
int   ElfReadDataSecs ( int  );
int   ElfReadSecHdrTbl ( int  );
void  ElfReadPgmHdrTbl ( int  );
int   ElfInsertDataSec ( const Elf32_Shdr * , int);
void  ElfFreeTables ( void );
void  ElfListTextSecs ( int, const Elf32_Ehdr * );
void  ElfListDataSecs ( const Elf32_Ehdr * );
void  ElfListFuncs ( void );
void  ElfFreeText ( void );
void  ElfFreeTextSecs ( struct text_secs * );
void  ElfFreeData ( void );
void  ElfFreeDataSecs ( struct data_secs * );
int ElfCheckExec ( const Elf32_Ehdr * );
#endif
        
#endif
