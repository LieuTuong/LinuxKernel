#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/uinstd.h>      //sys_call_table  __NR_*
#include<linux/fs.h>         // filp_open
#include<linux/slab.h>	    //kmalloc
#include<asm/paravirt.h>    // write_cr0
#include<asm/uaccess.h>    // get_fd, set_fd

#define PROC_V 		"/proc/version"
#define BOOT_PATH	"/boot/System.map-"
#define VERSION_LEN	256

unsigned long *sys_call_table = NULL;

asmlinkage ssize_t (*original_write)(int fd, const void *buf, size_t cnt);

static int find_sys_call_table(char *kern_ver)
{
	char system_map_entry[VERSION_LEN];
 	int i = 0;
	char *fileName;
	size_t len_filename = strlen(kern_ver) + strlen(BOOT_PATH) +1;
	struct file *f = NULL:
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs (KERNEL_DS);
	
	printk(KERN_INFO"Kernel version: %s\n" ,kern_ver);

	fileName = kmalloc(len_filename, GFP_KERNEL);
	if (fileName == NULL)
	{
		printk(KERN_INFO "kmalloc failed on allocation file name\n");
		return -1;
	}
	memset(fileName , 0 , len_filename);
	strncpy(fileName, BOOT_PATH, strlen(BOOT_PATH));
	strncat(fileName, kern_ver, strlen(kern_ver));

	f = filp_open(fileName, O_RDONLY, 0);
	if (IS_ERR(f) || ( f == NULL))
	{
		printk(KERN_INFO "Error opening System.map-<version> file: %s\n",fileName);
		return -1;
	}
	memset(system_map_entry, 0, VERSION_LEN);

	while( vfs_read(f, system_map_entry + i, 1, &f->f_pos)==1)
	{
		if (system_map_entry[i] == '\n' || i == VERSION_LEN )
		{
			i = 0;
			if (strstr(system_map_entry,"sys_call_table") != NULL)
			{
				char *sys_string;
				char *system_map_entry_ptr = system_map_entry;
		
				sys_string = kmalloc(VERSION_LEN, GFP_KERNEL);
				if (sys_string == NULL)
				{
					filp_close(f,0);
					set_fs(oldfs);
					kfree(fileName);
					return -1;
				}
				memset(sys_string, 0, VERSION_LEN);
				strncpy(sys_string, strsep(&system_map_entry_ptr," "),VERSION_LEN);
				kstroul(sys_string, 16, &sys_call_table);
				printk(KERN_INFO "syscall_table retrieved\n");
				kfree(sys_string);

				break;
			}
		memset(system_map_entry, 0, VERSION_LEN);
		continue;
		}
	i++;
	}
	filp_close(f,0);
	set_fs(oldfs);
	kfree(fileName);
	return 0;
}	



char *acquire_kernel_version (char *buf) {
    struct file *proc_version;
    char *kernel_version;
  
    
    mm_segment_t oldfs;
  
    
    oldfs = get_fs();
    set_fs (KERNEL_DS);
  
    proc_version = filp_open(PROC_V, O_RDONLY, 0);
    if (IS_ERR(proc_version) || (proc_version == NULL)) {
        return NULL;
    }
  
 
    memset(buf, 0, VERSION_LEN);
 
    vfs_read(proc_version, buf, VERSION_LEN, &(proc_version->f_pos));
  
 
    kernel_version = strsep(&buf, " ");
    kernel_version = strsep(&buf, " ");
    kernel_version = strsep(&buf, " ");
  
    filp_close(proc_version, 0);
    
    /*
     * Switch filesystem context back to user space mode
     */
    set_fs(oldfs);
  
    return kernel_version;
}
	

asmlinkage ssize_t (*new_write)(const char *pathName, int flags)
{
	

	printk(KERN_INFO"This is my hook_write()\n");

	
	int written_bytes = original_write(fd, buf, cnt);

	printk(KERN_INFO"Number of written bytes: %d\n,written_bytes);

	return written_bytes;
}


static int __init init_hook(void) {
    char *kernel_version = kmalloc(VERSION_LEN, GFP_KERNEL);
    printk(KERN_INFO"LOADED HOOK_WRITE\n");
    find_sys_call_table(acquire_kernel_version(kernel_version));
  
    printk(KERN_EMERG "Syscall table address: %p\n", sys_call_table);
    printk(KERN_EMERG "sizeof(unsigned long *): %zx\n", sizeof(unsigned long*));
    printk(KERN_EMERG "sizeof(sys_call_table) : %zx\n", sizeof(sys_call_table));
  
    if (sys_call_table != NULL) {
        write_cr0 (read_cr0 () & (~ 0x10000));
        original_write = (void *)sys_call_table[__NR_write];
        sys_call_table[__NR_write] = &new_write;
        write_cr0 (read_cr0 () | 0x10000);
        printk(KERN_EMERG "[+] onload: sys_call_table hooked\n");
    } else {
        printk(KERN_EMERG "[-] onload: syscall_table is NULL\n");
    }
  
    kfree(kernel_version);
  
    return 0;
}

static void __exit exit_hook(void) {
    if (sys_call_table != NULL) {
        write_cr0 (read_cr0 () & (~ 0x10000));
        syscall_table[__NR_write] = original_write;
        write_cr0 (read_cr0 () | 0x10000);
        printk(KERN_EMERG "[+] onunload: sys_call_table unhooked\n");
    } else {
        printk(KERN_EMERG "[-] onunload: syscall_table is NULL\n");
    }

    printk(KERN_INFO "REMOVE HOOK_WRITE\n");
}

module_init(onload);
module_exit(onunload);

MODULE_LICENSE("GPL");
