#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yaswanth Raj");
MODULE_DESCRIPTION("Hello world module");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello module loaded\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Hello module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);