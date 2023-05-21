/*
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uio.h>
#include <linux/uio_driver.h>
#include <linux/slab.h>
#include <linux/device.h>

#define CLASS_NAME "uio_logical_mem"
#define DEVICE_NAME "uio_mem_dev"

static struct 
{
	struct uio_info * info;
	struct class * uio_mem_class;
	struct device * dev;
	dev_t devt;
} moduleData;

int init_module(void)
{
	int rc;

	printk(KERN_INFO "Hello world!\n");

	moduleData.info = kmalloc(sizeof(struct uio_info), GFP_KERNEL);
	if (!moduleData.info)
		return -ENOMEM;
	else
		printk(KERN_INFO "UIO info allocated\n");

	moduleData.uio_mem_class = class_create(THIS_MODULE, CLASS_NAME);
	if (!moduleData.uio_mem_class)
	{
		printk(KERN_ERR "Failed to class_create()");
		return -1;
	}
	
	rc = alloc_chrdev_region(&moduleData.devt, 0, 1, DEVICE_NAME);
	if (rc != 0)
	{
		printk(KERN_ERR "Failed to alloc_chrdev_region: %d", rc);
		return -1;
	}

	moduleData.dev = device_create(moduleData.uio_mem_class, NULL, moduleData.devt, NULL, DEVICE_NAME);
	if (!moduleData.dev)
	{
		printk(KERN_ERR "Failed to device_create()");
		return -1;
	}

	moduleData.info->name = DEVICE_NAME;
	moduleData.info->version = "0.1.0";
	moduleData.info->irq = UIO_IRQ_NONE;
	
	moduleData.info->mem[0].name = "main";
	moduleData.info->mem[0].memtype = UIO_MEM_LOGICAL;
	moduleData.info->mem[0].addr = (phys_addr_t)(void *)get_zeroed_page(GFP_KERNEL | __GFP_COMP);
	moduleData.info->mem[0].size = PAGE_SIZE;

	rc = uio_register_device(moduleData.dev, moduleData.info);
	if (rc != 0)
	{
		printk(KERN_WARNING "Failed to register UIO: %d\n", rc);
		return rc;
	}
	else
		printk(KERN_INFO "UIO device registered\n");

	return 0;
}

void cleanup_module(void)
{
	uio_unregister_device(moduleData.info);
	free_page(moduleData.info->mem[0].addr);
	kfree(moduleData.info);

	device_destroy(moduleData.uio_mem_class, moduleData.dev->devt);
	class_destroy(moduleData.uio_mem_class);
	unregister_chrdev_region(moduleData.devt, 1);

	printk(KERN_INFO "Bye Bye cruel world\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxim Vechkanov");
