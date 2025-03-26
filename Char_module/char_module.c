#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yaswanth Raj");
MODULE_DESCRIPTION("Simple char driver");

static dev_t major_no;
static struct cdev char_cdev;
static struct class* char_class;
static struct device* char_device;

#define BUFFER_SIZE 512
char kernel_buffer[BUFFER_SIZE] = "Hello from kernel";
int kernel_index = 18;

static int open_my_char(struct inode* inode, struct file* file)
{
    pr_info("Char device opened\n");

    return 0;
}

static int close_my_char(struct inode *inode, struct file *file)
{
    pr_info("Char device closed\n");

    return 0;
}

static ssize_t read_my_char(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
    int remaining = kernel_index - *offset;
    if (remaining <=0)
    {
        return 0;
    }

    if (count > remaining)
    {
        count = remaining;
    }
    
    if (copy_to_user(user_buffer, kernel_buffer + *offset, count))
    {
        return -EINVAL;
    }

    *offset += count;
    kernel_index -= count;

    pr_info("from read function: %s\n", kernel_buffer);

    return count;
}

static ssize_t write_my_char(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
    if (count > BUFFER_SIZE)
    {
        count = BUFFER_SIZE;
    }

    if (copy_from_user(kernel_buffer, user_buffer, count))
    {
        return -EINVAL;
    }

    kernel_index += count;

    pr_info("from write function: %s\n", kernel_buffer);

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = open_my_char,
    .release = close_my_char,
    .read = read_my_char,
    .write = write_my_char,
};

static int __init char_module_init(void)
{
    int res = 0;
    pr_info("Loading char module\n");

    // allocate major number dynamically
    res = alloc_chrdev_region(&major_no, 0, 1, "My_char_device");
    if (res < 0)
    {
        return res;
    }

    //create class file in /sys/class/
    char_class = class_create(THIS_MODULE, "My_char_device");
    if (IS_ERR(char_class))
    {
        pr_err("failed to create class\n");
        unregister_chrdev_region(major_no, 1);
        return -EINVAL;
    }

    //create device file in /dev
    char_device = device_create(char_class, NULL, major_no, NULL, "My_char_device");
    if (IS_ERR(char_device))
    {
        pr_err("failed to create device\n");
        class_destroy(char_class);
        unregister_chrdev_region(major_no, 1);
        return -EINVAL;
    }

    //initialize cdev structure
    cdev_init(&char_cdev, &fops);
    char_cdev.owner = THIS_MODULE;
    res = cdev_add(&char_cdev, major_no, 1);
    if (res < 0)
    {
        pr_err("cdev failed to add to the kernel\n");
        device_destroy(char_class, major_no);
        class_destroy(char_class);
        unregister_chrdev_region(major_no, 1);
        return EINVAL;
    }

    pr_info("Major number %d\nMinor number: %d\n", MAJOR(major_no), MINOR(major_no));

    return 0;
}

static void __exit char_module_exit(void)
{
    cdev_del(&char_cdev);
    device_destroy(char_class, major_no);
    class_destroy(char_class);
    unregister_chrdev_region(major_no, 1);

    pr_info("unloading char module\n");
}

module_init(char_module_init);
module_exit(char_module_exit);