#include <linux/init.h>

#include <linux/version.h>

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/fs.h>

#include <linux/miscdevice.h>

#include <linux/sched.h>

#include <linux/gfp.h>

#include <linux/slab.h>

#include <linux/memory.h>

#include <linux/mm.h>
#include <asm/atomic.h>
#include <linux/mm_types.h>
#include <linux/list.h>
#include <paging.h>

atomic_t free_counter;
atomic_t fault_counter;
unsigned int num_pages;
static unsigned int demand_paging = 1;
module_param(demand_paging, uint, 0644);
struct track_list{
	struct list_head track_pfn;
	unsigned long pfn;
};
typedef struct tracker{
	atomic_t ref;
	struct list_head track;
}tracker;


static int

do_fault(struct vm_area_struct * vma,

         unsigned long           fault_address)

{

    printk(KERN_INFO "paging_vma_fault() invoked: took a page fault at VA 0x%lx\n", fault_address);

    return VM_FAULT_SIGBUS;

}



#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)

static int

paging_vma_fault(struct vm_area_struct * vma,

                 struct vm_fault       * vmf)

{

    unsigned long fault_address = (unsigned long)vmf->virtual_address

#else

static int

paging_vma_fault(struct vm_fault * vmf)



{
 
    struct vm_area_struct * vma = vmf->vma;

    unsigned long fault_address = (unsigned long)vmf->address;

#endif

    struct page * new_page;
    int flag_update;
    unsigned long pfn;
    unsigned long vaddr;
    tracker * tracker;
    struct track_list* temp;
    
    printk(KERN_INFO"page fault called, page fault address is %lx\n",fault_address);
    tracker=vma->vm_private_data;
    new_page=alloc_page(GFP_KERNEL);
    if(new_page==NULL) return VM_FAULT_OOM;
    pfn=page_to_pfn(new_page);
    vaddr=PAGE_ALIGN((unsigned long)vmf->address);
    flag_update=remap_pfn_range(vma,vaddr,pfn,PAGE_SIZE,vma->vm_page_prot);
    if(flag_update==0){
	temp=kmalloc(sizeof(struct track_list*), GFP_KERNEL);
	temp->pfn=pfn;
	list_add(&(temp->track_pfn),&(tracker->track));
    }else{
	return do_fault(vma,fault_address);
    }
    atomic_add(1,&fault_counter);
    return VM_FAULT_NOPAGE;


}



static void

paging_vma_open(struct vm_area_struct * vma)

{
    tracker * tracker;
    tracker=vma->vm_private_data;
    atomic_add(1,&(tracker->ref));
    printk(KERN_INFO "paging_vma_open() invoked\n");

}



static void

paging_vma_close(struct vm_area_struct * vma)

{
    tracker* tracker;
    struct list_head * pos,*q;
    struct track_list * temp;


    tracker=vma->vm_private_data;
    atomic_dec(&(tracker->ref));
    if(atomic_read(&tracker->ref)==0){
	printk(KERN_INFO "free memory\n");
	list_for_each_safe(pos,q,&(tracker->track)){
		temp=list_entry(pos,struct track_list,track_pfn);
		__free_page(pfn_to_page(temp->pfn));
		atomic_add(1,&free_counter);
		list_del(pos);
		kfree(temp);
	};
	kfree(tracker);
    }

    printk(KERN_INFO "paging_vma_close() invoked\n");

}



static struct vm_operations_struct

paging_vma_ops = 

{

    .fault = paging_vma_fault,

    .open  = paging_vma_open,

    .close = paging_vma_close

};

static int
my_get_order(unsigned int value)
{
    unsigned int shifts = 0;

    if (!value)
        return 0;

    if (!(value & (value - 1)))
        value--;

    while (value > 0) {
        value >>= 1;
        shifts++;
    }

    return shifts;
}

/* vma is the new virtual address segment for the process */

static int

paging_mmap(struct file           * filp,

            struct vm_area_struct * vma)

{

    /* prevent Linux from mucking with our VMA (expanding it, merging it 
 *
 *      * with other VMAs, etc.)
 *
 *           */
    int i;
    tracker * tracker;
    unsigned int order;
    struct page * pages;
    unsigned long pfn;
    int flag_update;
    struct track_list * temp;
    vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE

              | VM_DONTDUMP | VM_PFNMAP;



    /* setup the vma->vm_ops, so we can catch page faults */

    vma->vm_ops = &paging_vma_ops;
    
    tracker=kmalloc(sizeof(tracker),GFP_KERNEL);
    atomic_set(&(tracker->ref),1);
    INIT_LIST_HEAD(&(tracker->track));
    num_pages=(vma->vm_end-vma->vm_start)/PAGE_SIZE;
    if((vma->vm_end-vma->vm_start)%PAGE_SIZE>0) num_pages++;
    

    vma->vm_private_data=tracker;
    order=my_get_order(num_pages);

    if(demand_paging==0){
	pages=alloc_pages(GFP_KERNEL,order);
	if(pages==NULL){
		return -ENOMEM;
	}
	pfn=page_to_pfn(pages);
	flag_update=remap_pfn_range(vma,vma->vm_start,pfn,num_pages*PAGE_SIZE,vma->vm_page_prot);
    	if(flag_update==0){
		for(i=0;i<num_pages;i++){
			temp=kmalloc(sizeof(struct track_list*), GFP_KERNEL);
        	        temp->pfn=page_to_pfn(pages+i);
                	list_add(&(temp->track_pfn),&(tracker->track));
			//printk(KERN_INFO"pfn:%lx\n",temp->pfn);
		}
    	}else{
        	return -EFAULT;
    	}
    	atomic_add(1,&fault_counter);

    }





    printk(KERN_INFO "paging_mmap() invoked: new VMA for pid %d from VA 0x%lx to 0x%lx\n",

        current->pid, vma->vm_start, vma->vm_end);



    return 0;

}



static struct file_operations

dev_ops =

{

    .mmap = paging_mmap,

};



static struct miscdevice

dev_handle =

{

    .minor = MISC_DYNAMIC_MINOR,

    .name = PAGING_MODULE_NAME,

    .fops = &dev_ops,

};

/*** END device I/O **/



/*** Kernel module initialization and teardown ***/

static int

kmod_paging_init(void)

{

    int status;



    /* Create a character device to communicate with user-space via file I/O operations */

    status = misc_register(&dev_handle);

    if (status != 0) {

        printk(KERN_ERR "Failed to register misc. device for module\n");

        return status;

    }
    atomic_set(&fault_counter,0);
    atomic_set(&free_counter,0);


    printk(KERN_INFO "Loaded kmod_paging module\n");



    return 0;

}



static void

kmod_paging_exit(void)

{

    /* Deregister our device file */

    misc_deregister(&dev_handle);

    printk(KERN_INFO "free counter: %d",atomic_read(&free_counter));
    printk(KERN_INFO "fault counter: %d",atomic_read(&fault_counter));

    printk(KERN_INFO "Unloaded kmod_paging module\n");

}



module_init(kmod_paging_init);

module_exit(kmod_paging_exit);



/* Misc module info */

MODULE_LICENSE("GPL");


