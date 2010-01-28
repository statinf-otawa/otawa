
// adapted for gliss2, translated to C++

#ifndef __LOADER_H__
#define __LOADER_H__

#include <ppc/api.h>
#include <gel/gel.h>

/*==============================================================================
 * Juste a fiew ELF program header definitions
 *==============================================================================
 */
#define PT_LOAD 1 /* loadable segment type   */
#define PF_X    1 /* executable segment flag */

/*==============================================================================
 * int integrate_cluster_into_mem(gel_image_cluster_t *cluster, ppc_memory_t mem)
 *------------------------------------------------------------------------------
 * maps a cluster into the memory
 *------------------------------------------------------------------------------
 * input : -cluster : cluster to map in the memory
 *         -mem   : the memory to be mapped
 * ouput : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int integrate_cluster_into_mem(gel_memory_cluster_t *cluster, ppc_memory_t* mem);

/*==============================================================================
 * int gel_memory_to_gliss_memory()
 *------------------------------------------------------------------------------
 * maps a clusterized gel image to gliss memory
 *------------------------------------------------------------------------------
 * input : -image_info : the image info's image to map
 *         -mem   : the memory to be mapped
 * ouput : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int gel_image_to_gliss_memory(gel_memory_image_t* image_memory, ppc_memory_t* mem);

/*==============================================================================
 * int loader_free_gel_cluster_reference (ppc_memory_t* mem,
 *                                        gel_memory_cluster_t* gel_cluster)
 *------------------------------------------------------------------------------
 * free all reference to a gel allocated cluster in iss memory
 *------------------------------------------------------------------------------
 * input : -mem       : reference to iss memory
 *         -gel_cluster : reference to the gel cluster
 * output : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int loader_free_gel_cluster_reference (ppc_memory_t* mem, gel_memory_cluster_t* gel_cluster);

/*==============================================================================
 * int loader_free_gel_memory_reference (ppc_memory_t* mem, gel_image_t gel_image)
 *------------------------------------------------------------------------------
 * free all reference to gel allocated memory in iss memory
 *------------------------------------------------------------------------------
 * input : -mem       : reference to iss memory
 *         -gel_image : reference to gel image
 * output : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int loader_free_gel_memory_reference (ppc_memory_t* mem, gel_image_t* gel_image);

/*==============================================================================
 * void loader_init(ppc_state_t *state, ppc_memory_t *mem,void *args[])
 *------------------------------------------------------------------------------
 * load the elf file using GEL and map the GEL image to GLISS's memory format
 *------------------------------------------------------------------------------
 * input : -state : the processor state
 *         -mem   : the memory
 *         -param_list : parameter list for this external module. Index :
 *           0 -> int*   Argument count
 *           1 -> char** Arguments list (list of char* terminated by NULL value)
 *           2 -> char** Environment list
 *                (list of char* terminated by NULL value)
 *           3 -> char* gel plugins path
 * 			 4 -> gel_file_t *, GEL file handler if NULL command is given in
 * 				  the arguments.
 *------------------------------------------------------------------------------
 */
void loader_init(ppc_state_t *etat, ppc_memory_t *mem, void *loader_params[]);

/*==============================================================================
 * void loader_halt(void)
 *------------------------------------------------------------------------------
 * halt properly the loader module, doing all memory release needed
 *------------------------------------------------------------------------------
 */
void loader_halt(ppc_memory_t* memory_reference);

/*==============================================================================
 * uint32_t loader_get_brk_point(ppc_memory_t* memory, int memory_page_size))
 *------------------------------------------------------------------------------
 * process the brk point address, value that points to the end of the process
 * image data segment
 * this value is used by system calls (see system.c .h and sys_call.c .h)
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory where to find the brk point
 *         -memory_page_size : the size of memory pages
 * output : the brkpoint address
 *------------------------------------------------------------------------------
 */
uint32_t loader_get_brk_point(ppc_memory_t* memory, int memory_page_size);

/*==============================================================================
 * gel_file_t *loader_file(ppc_memory_t* memory);
 *------------------------------------------------------------------------------
 * return the GEL file handler used for loading the current program.
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory of the current program.
 * output : the GEL file handler.
 *------------------------------------------------------------------------------
 */
gel_file_t *loader_file(ppc_memory_t* memory);

/*==============================================================================
 * gel_image_t *loader_image(ppc_memory_t* memory);
 *------------------------------------------------------------------------------
 * return the GEL image handler used for loading the current program.
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory of the current program.
 * output : the GEL memory handler.
 *------------------------------------------------------------------------------
 */
gel_image_t *loader_image(ppc_memory_t *memory);

/* Constant to know endianness of the file. */
extern int Is_Elf_Little;

// execution environment
typedef struct ppc_env_t
{
	int argc;

	/* NULL terminated */
	char **argv;
	ppc_address_t argv_addr;

	/* NULL terminated */
	char **envp;
	ppc_address_t envp_addr;

	auxv_t *auxv;
	ppc_address_t auxv_addr;

	ppc_address_t stack_pointer;
} ppc_env_t;

#endif
