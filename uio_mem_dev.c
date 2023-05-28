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
#define LOCAL_MAX_DEVICES 10
#define MAX_UIO_NAME 64

static struct
{
	struct uio_info *info[LOCAL_MAX_DEVICES];
	struct class *uio_mem_class;
	struct device *dev[LOCAL_MAX_DEVICES];
	char uioNames[LOCAL_MAX_DEVICES][MAX_UIO_NAME];
	uint32_t numDevices;
	dev_t major;
	dev_t minor_start;

} moduleData;

int init_module(void)
{
	int rc;
	uint32_t devIdx;
	dev_t devt;

	memset(&moduleData, 0, sizeof(moduleData));

	moduleData.uio_mem_class = class_create(THIS_MODULE, CLASS_NAME);
	if (!moduleData.uio_mem_class)
	{
		printk(KERN_ERR "Failed to class_create()");
		return -1;
	}

	rc = alloc_chrdev_region(&devt, 0, LOCAL_MAX_DEVICES, DEVICE_NAME);
	if (rc != 0)
	{
		printk(KERN_ERR "Failed to alloc_chrdev_region: %d", rc);
		goto class_created;
	}

	moduleData.major = MAJOR(devt);
	moduleData.minor_start = MINOR(devt);

	for (devIdx = 0; devIdx < 5; ++devIdx)
	{
		moduleData.dev[devIdx] = device_create(moduleData.uio_mem_class, NULL, MKDEV(moduleData.major, moduleData.minor_start + devIdx), NULL, DEVICE_NAME "%u", devIdx);
		if (!moduleData.dev[devIdx])
		{
			printk(KERN_ERR "Failed to device_create()");
			return -1;
		}

		moduleData.info[devIdx] = kmalloc(sizeof(struct uio_info), GFP_KERNEL);
		if (!moduleData.info[devIdx])
			return -ENOMEM;
		// else
		// 	printk(KERN_INFO "UIO info allocated\n");

		memset(moduleData.info[devIdx], 0, sizeof(struct uio_info));

		snprintf(moduleData.uioNames[devIdx], MAX_UIO_NAME, DEVICE_NAME "_%u", devIdx);

		moduleData.info[devIdx]->name = moduleData.uioNames[devIdx];
		moduleData.info[devIdx]->version = "0.1.0";
		moduleData.info[devIdx]->irq = UIO_IRQ_NONE;

		moduleData.info[devIdx]->mem[0].name = "main";
		moduleData.info[devIdx]->mem[0].memtype = UIO_MEM_LOGICAL;
		moduleData.info[devIdx]->mem[0].addr = (phys_addr_t)(void *)get_zeroed_page(GFP_KERNEL | __GFP_COMP);
		moduleData.info[devIdx]->mem[0].size = PAGE_SIZE;

		rc = uio_register_device(moduleData.dev[devIdx], moduleData.info[devIdx]);
		if (rc != 0)
		{
			printk(KERN_WARNING "Failed to register UIO: %d\n", rc);
			return rc;
		}
		else
			printk(KERN_INFO "UIO device registered\n");

		++moduleData.numDevices;
	}

	return 0;

// device_created:

// 	device_destroy(moduleData.uio_mem_class, moduleData.dev->devt);

class_created:

	class_destroy(moduleData.uio_mem_class);

	return rc;
}

void cleanup_module(void)
{
	uint32_t devIdx;
	for (devIdx = 0; devIdx < moduleData.numDevices; ++devIdx)
	{
		uio_unregister_device(moduleData.info[devIdx]);
		free_page(moduleData.info[devIdx]->mem[0].addr);
		kfree(moduleData.info[devIdx]);
		device_destroy(moduleData.uio_mem_class, moduleData.dev[devIdx]->devt);
	}

	moduleData.numDevices = 0;

	class_destroy(moduleData.uio_mem_class);
	unregister_chrdev_region(MKDEV(moduleData.major, moduleData.minor_start), LOCAL_MAX_DEVICES);

	printk(KERN_INFO "Bye Bye cruel world\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maxim Vechkanov");
