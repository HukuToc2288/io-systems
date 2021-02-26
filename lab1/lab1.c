#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>

static dev_t first;
static struct cdev charDevice;
static struct class *cl;
static struct proc_dir_entry *procEntry;

struct LinkedIntList {
    int spacesCount;
    struct LinkedIntList *nextEntry;
};

struct LinkedIntList *firstEntry = NULL;
struct LinkedIntList *lastEntry = NULL;
struct LinkedIntList *currentEntry = NULL;

void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[]) {
    int i, sign;

    if ((sign = n) < 0)  /* записываем знак */
        n = -n;          /* делаем n положительным числом */
    i = 0;
    do {       /* генерируем цифры в обратном порядке */
        s[i++] = n % 10 + '0';   /* берем следующую цифру */
    } while ((n /= 10) > 0);     /* удаляем */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
    s[i++] = '\n';
    s[i] = '\0';
}

static int charDeviceOpen(struct inode *i, struct file *f) {
    //printk(KERN_INFO "Driver: open()\n");
    return 0;
}

static int charDeviceClose(struct inode *i, struct file *f) {
   // printk(KERN_INFO "Driver: close()\n");
    return 0;
}

static ssize_t charDeviceRead(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    // size_t len = strlen(THIS_MODULE->name);
    printk(KERN_DEBUG "%s: Char device read", THIS_MODULE->name);
    struct LinkedIntList *tempEntry = firstEntry;
    while (tempEntry != NULL) {
        char data[10];
        itoa(tempEntry->spacesCount, data);
        printk(KERN_INFO "%s: %d\n", THIS_MODULE->name, currentEntry->spacesCount);
        tempEntry = tempEntry->nextEntry;
    }
    return 0;
}

static ssize_t procWrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "%s: Attempt to write proc file", THIS_MODULE->name);
    return 1;
}

static ssize_t procRead(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    if (currentEntry == NULL) {
        currentEntry = firstEntry;
        return 0;
    }
    char data[10];
    itoa(currentEntry->spacesCount, data);
    currentEntry = currentEntry->nextEntry;
    size_t datalen = strlen(data);

    if (count > datalen) {
        count = datalen;
    }

    if (copy_to_user(ubuf, data, count)) {
        return -EFAULT;
    }

    return count;
}

static struct file_operations procFops = {
        .owner = THIS_MODULE,
        .read = procRead,
        .write = procWrite,
};

static ssize_t charDeviceWrite(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    //printk(KERN_INFO "Driver: write()\n");
    int spacesCount = 0;

    char line[len];
    if (copy_from_user(line, buf, len)){
        return -EFAULT;
    }

    for (int i = 0; i < len; ++i) {
        if (line[i] == ' ')
            spacesCount++;
    }

    if (firstEntry == NULL) {
        firstEntry = (struct LinkedIntList *) (kmalloc(sizeof(struct LinkedIntList), GFP_KERNEL));
        firstEntry->spacesCount = spacesCount;
        firstEntry->nextEntry = NULL;
        lastEntry = firstEntry;
        currentEntry = lastEntry;
    } else {
        struct LinkedIntList *tempEntry = (struct LinkedIntList *) (kmalloc(sizeof(struct LinkedIntList), GFP_KERNEL));
        tempEntry->spacesCount = spacesCount;
        tempEntry->nextEntry = NULL;
        lastEntry->nextEntry = tempEntry;
        lastEntry = tempEntry;
    }
    return len;
}

static struct file_operations charDeviceFops =
        {
                .owner = THIS_MODULE,
                .open = charDeviceOpen,
                .release = charDeviceClose,
                .read = charDeviceRead,
                .write = charDeviceWrite
        };

static int __init moduleInit(void) {
    printk(KERN_INFO "%s: Henlo!\n", THIS_MODULE->name);
    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0) {
        return -1;
    }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
        unregister_chrdev_region(first, 1);
        return -1;
    }
    if (device_create(cl, NULL, first, NULL, "var4") == NULL) {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }
    cdev_init(&charDevice, &charDeviceFops);
    if (cdev_add(&charDevice, first, 1) == -1) {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    //make proc file
    procEntry = proc_create("var4", 0444, NULL, &procFops);
    printk(KERN_INFO "%s: Module loaded\n", THIS_MODULE->name);
    return 0;
}

static void __exit moduleExit(void) {
    cdev_del(&charDevice);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "%s: Bye!\n", THIS_MODULE->name);
    proc_remove(procEntry);
}

module_init(moduleInit);

module_exit(moduleExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nykyta Czernyszczew and Grzegorz Komarow");
MODULE_DESCRIPTION("The first kernel module");