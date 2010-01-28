
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <string>

#include <gel/gel.h>

#include <ppc/api.h>
#include <ppc/macros.h>
#include <ppc/mem.h>

#include "gel_loader.h"
#include "mem_intern.h"


#define STACK_SIZE		0x10000     /* 64 ko */
#define STACK_ADDRESS	0x80000000
#define CLUSTER_SIZE	4096        /* 4 ko */

/* Constant to know endianness of the file. */
int Is_Elf_Little = 0;

/* temporary module reference to memory */
/*static ppc_memory_t *memory_reference;*/

/* struct designed to be referenced by the image_link field of memory64_t */
typedef struct
{
	gel_image_t *gel_image;
	gel_file_t *gel_file;
} gel_memory_link_t;

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
int integrate_cluster_into_mem(gel_memory_cluster_t *cluster, ppc_memory_t* mem)
{
	gel_memory_cluster_info_t cluster_info;
	ppc_address_t address;
	ppc_address_t address_limit;
	secondary_memory_hash_table_t *secondary_hash_table;
	uint32_t hash_code;
	memory_page_table_entry_t* page_entry;

	/* fetch infos about the cluster to integrate */
	gel_memory_cluster_infos(cluster,&cluster_info);

	/* computes the address limit cluster */
	address_limit = cluster_info.vaddr + cluster_info.size -1;

	/* for each page, map it to the memory */
	for (address = cluster_info.vaddr;
	     address < address_limit ;
	     address += MEMORY_PAGE_SIZE)
	{
		/* fetch the secgondary hashmap */
		secondary_hash_table = mem_get_secondary_hash_table(mem, address);
		if (secondary_hash_table == NULL)
		{
			return -1;
		}

		/* create the page table entry */
		page_entry = (memory_page_table_entry_t *) malloc(sizeof(memory_page_table_entry_t));

		if (page_entry == NULL)
	  	{
	  		return -1;
	  	}
		/* fill in the page entry */
		page_entry->addr = address;
		page_entry->storage = cluster_info.raddr + (address - cluster_info.vaddr);

		/* link to the list of pages in the page entry */
		hash_code = mem_hash2(address);
		page_entry->next = secondary_hash_table->pte[hash_code];
		secondary_hash_table->pte[hash_code] = page_entry;
	}

	return 0;
}

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
int gel_image_to_gliss_memory(gel_memory_image_t *image_memory, ppc_memory_t *mem)
{
	gel_memory_image_info_t memory_info;
	int i;

	/* fetch infos on the memory */
	gel_memory_image_infos(image_memory, &memory_info);

	/* integrate each clusters into memory*/
	for (i=0; i < memory_info.nb_clusters; i++)
	{
		if ( integrate_cluster_into_mem(memory_info.clusters[i], mem) != 0)
		{
			return -1;
		}
	}

	return 0;
}



/*==============================================================================
 * void loader_init(state_t *state, memory_t *mem,void *param_list[])
 *------------------------------------------------------------------------------
 * load the elf file using GEL and map the GEL image to GLISS's memory format
 *------------------------------------------------------------------------------
 * input : -state : the processor state
 *         -mem   : the memory
 *         -param_list : parameter list for this external module. Standard and
 * 		extended versions available
 * 		 Index 0 = NULL to indicate EXTENDED version (in that case,
 *			offset every index by one)
 *           0 -> char** Arguments list (list of char* terminated by NULL value)
 *           1 -> char** Environment list
 *                (list of char* terminated by NULL value)
 *           2 -> char* gel plugins path
 *           3 -> char**  list of library search path terminated by NULL
 *                ( defaut : "./", "/lib","/usr/lib", "/usr/local/lib",
 *                           "/usr/X11R6/lib" )
 * 			 4 -> if the first argument is null, this contains the gel_file_t *
 * 				  of the used binary,
 * 			 5 -> u32_t *, flags to use in the GEL environment.
 *			 6 -> EXTENDED only - contains the address of the expected stack top
 *------------------------------------------------------------------------------
 */
void loader_init(ppc_state_t *state, ppc_memory_t *mem, void *param_list[])
{
	gel_file_t *gel_file; /* gel file descriptor */
	gel_env_t *gel_env; /* gel environment */
	gel_image_t *gel_image; /* gel process memory image */
	gel_image_info_t image_info; /* information sur l'image cree par gel */
	ppc_memory_t *memory;
	int param_offset = 0;

	/* checking param list */
	if (param_list==NULL || param_list[1] == NULL)
		throw "Error: loader_init(..), invalid loader parameters.\n";

	if (param_list[0] == NULL)
	{
		/* EXTENDED parameters */
		param_offset = 1;
	}

	/* opening file */
	if( ! *(char**)param_list[0 + param_offset])
	{
		gel_file = (gel_file_t *)param_list[4 + param_offset];
		assert(gel_file);
		gel_file_info_t infos;
		gel_file_infos(gel_file, &infos);
		*(char**)param_list[0 + param_offset] = infos.filename;
	}
	else
	{
		gel_file = gel_open(*(char**)param_list[0 + param_offset], (char*)param_list[2 + param_offset], 0);
		if (gel_file == NULL)
		{
			std::string c = "Error : loader_init, cannot load ELF file : ";
			c += gel_strerror();
			c += "\n";
			throw c;
		}
    	}

	/* Setting endianness */
	if(gel_file_endianness(gel_file) == GEL_LITTLE_ENDIAN)
		Is_Elf_Little = 1;
	else
		Is_Elf_Little = 0;

	/* creating environment */
	gel_env = gel_default_env();
	if (gel_env == NULL)
	{
	    gel_close(gel_file);
	    return;
	}
	/* filling in environment */
	gel_env->argv = (char **)param_list[0 + param_offset];
	gel_env->envp = (char **)param_list[1 + param_offset] ;
	gel_env->stackaddr = STACK_ADDRESS;
	gel_env->clustersize = CLUSTER_SIZE;
	gel_env->stacksize = STACK_SIZE;
	gel_env->flags = GEL_ENV_CLUSTERISED_MEMORY;
	gel_env->stacktop = NULL;
	if (param_list[5 + param_offset])
		gel_env->flags |= *(u32_t *)param_list[5];
	if (param_list[3 + param_offset] != NULL)
	{
		gel_env->libpath = (char **)param_list[3 + param_offset];
	}
	if (param_offset == 1)
	{
		if(param_list[6] != NULL)
		{
			gel_env->stacktop = (vaddr_t)param_list[7];
		}
	}

	/* loading image */
	gel_image = gel_image_load(gel_file, gel_env, GEL_IMAGE_CLOSE_LIBS);

	/* NOTE : don't close exec file to fetch data segements address and size
	 *	  (required by system)*/
	if (gel_image == NULL)
	{
		gel_close(gel_file);
		free(gel_env);
		std::string c = "Error : loader_init, cannot load ELF image : ";
		c += gel_strerror();
		c += "\n";
		throw c;
		return;
	}

	memory = (memory_64_t*) mem;

	/* fill in the memory link */
	memory->image_link = malloc(sizeof(gel_memory_link_t));
	if (memory->image_link == NULL)
	{
		gel_close(gel_file);
		free(gel_env);
		throw "Error : loader_init, not enouth memory";
	}
	((gel_memory_link_t*)memory->image_link)->gel_image = gel_image;
	((gel_memory_link_t*)memory->image_link)->gel_file = gel_file;

	/* fetching image info */
	gel_image_infos(gel_image,&image_info);

	/* convert gel image to gliss memory */
	if (gel_image_to_gliss_memory(image_info.memory, mem) != 0)
		throw "Error : loader_init, error while converting gel to gliss memory";

	/* temporary reference : all global reference should be removed in future */
	/*memory_reference = mem;*/

	/* set the initial state -- fix to comply with System V ABI */
	PPC_GPR[1] = gel_env->sp_return; /*image_info.stack_pointer;*/
    	PPC_GPR[3] = gel_env->argc_return;
	PPC_GPR[4] = gel_env->argv_return;
	PPC_GPR[5] = gel_env->envp_return;
	PPC_GPR[6] = gel_env->auxv_return;
	PPC_GPR[7] = 0;

	/* set the Pc */
	PPC_CIA = image_info.ventry;
	PPC_NIA = PPC_CIA;
	return;
}

/*==============================================================================
 * int loader_free_gel_cluster_reference (memory_t* mem,
 *                                        gel_memory_cluster_t* gel_cluster)
 *------------------------------------------------------------------------------
 * free all reference to a gel allocated cluster in iss memory
 *------------------------------------------------------------------------------
 * input : -mem       : reference to iss memory
 *         -gel_cluster : reference to the gel cluster
 * output : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int loader_free_gel_cluster_reference (ppc_memory_t* mem,
                                       gel_memory_cluster_t* gel_cluster)
{
	gel_memory_cluster_info_t cluster_info;
	ppc_address_t address;
	uint32_t hash_code;
	ppc_address_t address_limit;
	secondary_memory_hash_table_t* secondary_hash_table;
	memory_page_table_entry_t* page_entry;
	memory_page_table_entry_t** last_page_table_entry_ptr; /* pointer used to
	                                                          simplify the
	                                                          page entry
	                                                          unlink */

	/* fetch cluster info */
	gel_memory_cluster_infos(gel_cluster, &cluster_info);

	/* computes the address limit cluster */
	address_limit = cluster_info.vaddr + cluster_info.size -1;

	/* for each page, map it to the memory */
	for (address = cluster_info.vaddr; address < address_limit ; address += MEMORY_PAGE_SIZE)
	{
		/* fetch the secgondary hashmap */
		secondary_hash_table = mem_get_secondary_hash_table(mem, address);
		if (secondary_hash_table == NULL)
		{
			return -1;
		}

		hash_code = mem_hash2(address);

		/* fetch the page table entry */
		page_entry = secondary_hash_table->pte[hash_code];
		last_page_table_entry_ptr =  &(secondary_hash_table->pte[hash_code]);

	  	/* search the right page table entry in the list */
		while (page_entry != NULL && page_entry->addr != address)
		{
			last_page_table_entry_ptr = &(page_entry->next);
			page_entry = page_entry->next;
		}

		/* unlink and free the page entry */
		if (page_entry != NULL)
		{
			/* unlink */
			*last_page_table_entry_ptr = page_entry->next;
			/* free the page entry */
			free(page_entry);
		}
		else
		{
			/* an error occured */
			return -1;
		}

	}

	return 0;
}

/*==============================================================================
 * int loader_free_gel_memory_reference (memory_t* mem, gel_image_t gel_image)
 *------------------------------------------------------------------------------
 * free all reference to gel allocated memory in iss memory
 *------------------------------------------------------------------------------
 * input : -mem       : reference to iss memory
 *         -gel_image : reference to gel image
 * output : 0 if everything has been well done, else -1
 *------------------------------------------------------------------------------
 */
int loader_free_gel_memory_reference (ppc_memory_t* mem, gel_image_t* gel_image)
{
	gel_image_info_t image_info;
	gel_memory_image_info_t memory_info;
	int i;

	/* fetch image info */
	gel_image_infos(gel_image, &image_info);
	gel_memory_image_infos(image_info.memory, &memory_info);

	/* for each cluster, freeing the memory table entry and the secondary
	 * hash table reference */
	for (i = 0 ; i < memory_info.nb_clusters ; i++)
	{
		if (loader_free_gel_cluster_reference(mem, memory_info.clusters[i]) != 0)
		{
			return -1;
		}
	}

	return 0;
}

/*==============================================================================
 * void loader_halt(void)
 *------------------------------------------------------------------------------
 * halt properly the loader module, doing all memory release needed
 *------------------------------------------------------------------------------
 */
void loader_halt(ppc_memory_t* memory_reference)
{
	/*ppc_memory_t *memory;
	memory = (ppc_memory_t *) memory_reference;*/

	/* erase all reference to gel memory */
	if (loader_free_gel_memory_reference (memory_reference, ((gel_memory_link_t*)memory_reference->image_link)->gel_image) != 0)
		throw "Error : error while freeing gel memory references\n";

	/* then close gel image */
	gel_image_close(((gel_memory_link_t*)memory_reference->image_link)->gel_image);

	/* free the link to image */
	free(memory_reference->image_link);

	memory_reference->image_link = NULL;
	/*memory_reference = NULL;*/
}

/*==============================================================================
 * uint32_t loader_get_brk_point(memory_t* memory, int memory_page_size))
 *------------------------------------------------------------------------------
 * process the brk point address, value that points to the end of the process
 * image data segment
 * this value is used by system calls (see system.c .h and sys_call.c .h)
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory where to find the brk point
 *         -memory_page_size : the size of memory pages
 * output : the brkpoint address or 0 if not found
 *------------------------------------------------------------------------------
 */
uint32_t loader_get_brk_point(ppc_memory_t *memory, int memory_page_size)
{
	gel_file_info_t file_info;
	gel_memory_link_t* memory_link;
	uint16_t i;
	gel_prog_t* segment ;
	gel_prog_info_t segment_info;

	segment = NULL;
	memory_link = static_cast<gel_memory_link_t *>(memory->image_link);

	/* fetch file info */
	gel_file_infos(memory_link->gel_file, &file_info);

	/* search the data segment of executable */
	uint32_t brk = 0;
	//fprintf(stderr, "GLISS LOADER %d\n", file_info.prognum);
	for (i = 0; i < file_info.prognum; i++)
	{
		/* fetch segment number i*/
		segment = gel_getprogbyidx(memory_link->gel_file, i);
		/* fetch infos about this segment */
		gel_prog_infos(segment,&segment_info);

		/* search segement loadable */
		if (segment_info.type == PT_LOAD)
		{
			/*fprintf(stderr, "GLISS LOADER %p (%08x)\n",
				segment_info.vaddr, segment_info.size);*/
			uint32_t new_brk = segment_info.vaddr + segment_info.memsz;
			if (new_brk > brk)
				brk = new_brk;
		}
	}

	/* Return result */
	return (brk + memory_page_size - 1) & ~(memory_page_size - 1);
}


/*==============================================================================
 * gel_file_t *loader_file(memory_t* memory);
 *------------------------------------------------------------------------------
 * return the GEL file handler used for loading the current program.
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory of the current program.
 * output : the GEL file handler.
 *------------------------------------------------------------------------------
 */
gel_file_t *loader_file(ppc_memory_t* memory_reference)
{
	return ((gel_memory_link_t*)memory_reference->image_link)->gel_file;
}


/*==============================================================================
 * gel_image_t *loader_image(memory_t* memory);
 *------------------------------------------------------------------------------
 * return the GEL image handler used for loading the current program.
 *------------------------------------------------------------------------------
 * input : -memory : pointer to the memory of the current program.
 * output : the GEL image handler.
 *------------------------------------------------------------------------------
 */
gel_image_t *loader_image(ppc_memory_t* memory_reference)
{
	return ((gel_memory_link_t*)memory_reference->image_link)->gel_image;
}


