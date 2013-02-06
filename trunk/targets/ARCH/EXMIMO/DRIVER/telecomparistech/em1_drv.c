
#include "em1_drv.h"

MODULE_AUTHOR("Alexandre Becoulet, Cerdan Sebastien");
MODULE_DESCRIPTION("Driver for Express-Mimo 1 board");
MODULE_SUPPORTED_DEVICE("Express-Mimo 1");
MODULE_LICENSE("GPL");


static int major = 256;
struct em1_private_s *em1_devices[MAX_EM1_DEVICES] = {};

int em1_user_op_enter(struct em1_private_s *pv, wait_queue_t *wait,
		      wait_queue_head_t *wh, int busy_flag, int new_state)
{
	add_wait_queue(wh, wait);

	spin_lock_irq(&pv->lock);

	/* wait for other tasks to leave */
	while (pv->busy & busy_flag)
	{
		if (new_state == TASK_INTERRUPTIBLE &&
		    signal_pending(current))
			return 1;
		set_current_state(new_state);
		spin_unlock_irq(&pv->lock);
		schedule();
		spin_lock_irq(&pv->lock);
	}

	pv->busy |= busy_flag;

	return 0;
}

void em1_user_op_leave(struct em1_private_s *pv, wait_queue_t *wait,
		       wait_queue_head_t *wh)
{
	set_current_state(TASK_RUNNING);
	spin_unlock_irq(&pv->lock);

	remove_wait_queue(wh, wait);

	// wake other waiting tasks
	wake_up_all(wh);
}

static int em1_open(struct inode *inode, struct file *file)
{
	int i;
	//FIXME Lock list access

	for (i = 0; i < MAX_EM1_DEVICES; i++)
	{
		struct em1_private_s *pv = em1_devices[i];

		if (pv != NULL && i == iminor(inode))
		{
			struct em1_file_s *fpv = kmalloc(sizeof(struct em1_file_s), GFP_KERNEL);
			if (!fpv)
				return -ENOMEM;

			fpv->pv = pv;
			file->private_data = fpv;
			fpv->read.addr = 0;
			fpv->write.addr = 0;
			return 0;
		}
	}

	return -ENODEV;
}

static int em1_release(struct inode *inode, struct file *file)
{
	struct em1_file_s *fpv = file->private_data;

	em1_unlock_user_page(&fpv->read);
	em1_unlock_user_page(&fpv->write);
	kfree(fpv);

	printk(KERN_DEBUG "exmimo1 : release()\n");
	return 0;
}

static const struct file_operations em1_fops = {
	.read		= em1_read,
	.write		= em1_write,
	.open		= em1_open,
	.release	= em1_release,
	.ioctl		= em1_ioctl,
};

/* Supported devices */
static struct pci_device_id em1_pci_tbl[] = {
	{ XILINX_VENDOR, XILINX_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,} 
};

MODULE_DEVICE_TABLE(pci, em1_pci_tbl);


static struct pci_driver em1_driver = {
	.name		= "exmimo1",
	.id_table	= em1_pci_tbl,
	.probe		= em1_probe,
	.remove		= em1_remove,
};

static int __init em1_init(void)
{
	int ret;
        printk(KERN_DEBUG "exmimo1 : Hello world!\n");

        ret = pci_register_driver(&em1_driver);	
	if (ret < 0) 
	{
          printk(KERN_ERR "exmimo1 : Error when registering PCI driver\n");
          goto err;
        }

	ret = register_chrdev(major, "exmimo1", &em1_fops);
	if (ret < 0) 
        {
          printk(KERN_ERR "exmimo1 : Error when registering char device\n");
          goto err;
        }
err:
        return ret;
}

static void __exit em1_cleanup(void)
{
        printk(KERN_DEBUG "exmimo1 : Goodbye world!\n");
	unregister_chrdev(major, "exmimo1");
        pci_unregister_driver(&em1_driver);

}

module_init(em1_init);
module_exit(em1_cleanup);

/*
 * Local Variables:
 * c-file-style: "linux"
 * indent-tabs-mode: t
 * tab-width: 8
 * End:
 */
