#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/unistd.h>
//#include<asm/semaphore.h>
#include<asm/cacheflush.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<asm/paravirt.h>
#include<linux/sched.h>
#include<linux/highmem.h>
#include<linux/syscalls.h>

void **sys_call_table;

asmlinkage int (*original_call)(const char*,int);

asmlinkage int hook_open(const char* file, int flag)
{
	printk(KERN_INFO"This is my hook_open\n");
	//printk(KERN_INFO"THREAD NAME: %s\n", current->comm);
	printk(KERN_INFO"file open: %s\n", file);
	return original_call(file, flag);
}

static void allow_writing(void)
{
	write_cr0(read_cr0() & (~ 0x10000));
}

static void disallow_writing (void)
{
	write_cr0(read_cr0() | 0x10000);
}

/*int make_rw(long unsigned int addr)
{
	/*unsigned int level;
	pte_t *pte=lookup_address(addr,&level);
	if(pte->pte & ~_PAGE_RW)
	{
		pte->pte|=_PAGE_RW;
	}
	return 0;
	//return set_memory_rw(PAGE_ALIGN(addr) - PAGE_SIZE,1);
}


int make_ro(long unsigned int addr)
{
	/*unsigned int level;
	pte_t *pte = lookup_address(addr,&level);
	pte->pte=pte->pte&~_PAGE_RW;
	return 0;
	return set_memory_ro(PAGE_ALIGN(addr) - PAGE_SIZE,1);
}*/
static int __init init_hook(void)
{
	
	sys_call_table = (void*)0xffffffff81a001c0;
	original_call = sys_call_table[__NR_open];
	allow_writing();
	//make_rw((unsigned long)sys_call_table);
	sys_call_table[__NR_open] = hook_open;
	disallow_writing();
	printk(KERN_INFO" LOADED MY HOOK\n");
	return 0;
}


static void __exit exit_hook(void)
{
	
	allow_writing();
	
	sys_call_table[__NR_open] = original_call;
	disallow_writing();
	//make_ro((unsigned long)sys_call_table);
	printk(KERN_INFO"REMOVED MY HOOK\n");
}


module_init(init_hook);
module_exit(exit_hook);
