#include <linux/fs.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <asm-generic/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yaswanth Raj");
MODULE_DESCRIPTION("Simple ioctl driver");

#define MAGIC_NO 'M'
#define IOCGET _IOR(MAGIC_NO, 1, int)
#define IOCSET _IOR(MAGIC_NO, 2, int)
#define IOCTOGGLE _IOR(MAGIC_NO, 3, int)

static dev_t major_no;
static struct cdev ioctl_cdev;
static struct class* ioctl_class;
static struct device* ioctl_device;

static int kernel_var = 99;

static long my_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        case IOCGET:
            if (copy_to_user((int __user*)arg, &kernel_var, sizeof(int)))
            {
                pr_err("failed to get from kernel\n");
                return -EFAULT;
            }
            pr_info("IOCGET: %d", kernel_var);
            break;
        
        case IOCSET:
            if (copy_from_user(&kernel_var, (int __user*)arg, (sizeof(int))))
            {
                pr_err("failed to get from user\n");
                return -EFAULT;
            }
            pr_info("IOCSET: %d", kernel_var);
            break;
        
        case IOCTOGGLE:
            kernel_var ^= 1; // toggle lsb
            pr_info("IOCTOGGLE: %d", kernel_var);
            break;
        
        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = my_ioctl,
};

static int ioctl_dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);

    return 0;
}

static int __init ioctl_module_init(void)
{
    int res = 0;

    // allocate major number dynamically
    res = alloc_chrdev_region(&major_no, 0, 1, "my_ioctl_chardev");
    if (res < 0)
    {
        return res;
    }

    //create class file in /sys/class/
    ioctl_class = class_create(THIS_MODULE, "my_ioctl_class");
    if (IS_ERR(ioctl_class))
    {
        pr_err("failed to create class\n");
        unregister_chrdev_region(major_no, 1);
        return -EFAULT;
    }
    ioctl_class->dev_uevent = ioctl_dev_uevent;

    //create device file in /dev
    ioctl_device = device_create(ioctl_class, NULL, major_no, NULL, "my_ioctl_device");
    if (IS_ERR(ioctl_device))
    {
        pr_err("failed to create device\n");
        class_destroy(ioctl_class);
        unregister_chrdev_region(major_no, 1);
        return -EFAULT;
    }

    //initialize cdev structure
    cdev_init(&ioctl_cdev, &fops);
    ioctl_cdev.owner = THIS_MODULE;
    res = cdev_add(&ioctl_cdev, major_no, 1);
    if (res < 0)
    {
        pr_err("cdev failed to add to the kernel\n");
        device_destroy(ioctl_class, major_no);
        class_destroy(ioctl_class);
        unregister_chrdev_region(major_no, 1);
        return -EFAULT;
    }
    pr_info("Loaded: ioctl module\n");
    pr_info("Major number %d\nMinor number: %d\n", MAJOR(major_no), MINOR(major_no));

    return 0;
}

static void __exit ioctl_module_exit(void)
{
    // remove all the created file when unloading
    cdev_del(&ioctl_cdev);
    device_destroy(ioctl_class, major_no);
    class_destroy(ioctl_class);
    unregister_chrdev_region(major_no, 1);

    pr_info("unloaded: ioctl module\n");
}

module_init(ioctl_module_init);
module_exit(ioctl_module_exit);