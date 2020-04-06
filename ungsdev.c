/*
 * ungsdev.c: Crea un chardev que se le envia datos y se los puede leer luego.
 *
 * Basado en chardev.c desde TLDP.org's LKMPG book.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "UNGS"
#define BUF_LEN 80

/*
 * Variables globales declaradas como estáticas.
 */

static int Major;
static int Device_Open = 0;

static char msg[BUF_LEN];
static char *msg_Ptr;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * Se invoca cuando se carga el módulo.
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    printk(KERN_ALERT "Char-Device registrado con exito! Major = %d.\n", Major);

    printk(KERN_INFO "Crear un dev_file con \n");
    printk(KERN_INFO "sudo rm /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "sudo mknod /dev/%s c %d 0\n", DEVICE_NAME, Major);
    printk(KERN_INFO "sudo chmod 666 /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "Eliminar el /dev y el modulo al termina.\n");

    return SUCCESS;
}

/*
 * Se invoca cuando el módulo se descarga.
 */
void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Se invoca cuando un proceso intenta abrir el archivo device.
 * cat /dev/UNGS
 */
static int device_open(struct inode *inode, struct file *filp)
{
    if (Device_Open)
        return -EBUSY;
    Device_Open++;

    sprintf(msg, "%s",msg);

    msg_Ptr = msg;

    return SUCCESS;
}

/*
 * Se invoca cuando un proceso cierra el archivo device.
 */
static int device_release(struct inode *inode, struct file *filp)
{

    Device_Open--;

    return SUCCESS;
}

/*
 * Se invoca cuando un proceso, que ya abrió el archivo device, intenta leerlo.
 */
static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t *offset)
{

        int bytes_read = 0;

        /*printk(KERN_INFO "DEBUG: device_read: %s\n",buffer);*/

        if (*msg_Ptr == 0)
                return 0;

        while (length && *msg_Ptr) {

                put_user(*(msg_Ptr++), buffer++);

                length--;
                bytes_read++;
        }

    return bytes_read;
}

/*
 * Se invoca cuando se escribe en el archivo device:
 * echo "Hola Mundo" > /dev/UNGS
 */
static ssize_t device_write(struct file *file,
             const char *buffer, size_t length, loff_t * offset){

        int i;

        for (i = 0; i < length && i < BUF_LEN; i++){
                        get_user(msg[i], buffer + i);
        }

        /*printk(KERN_INFO "DEBUG device_write: :%s\n", msg);*/
        msg[length] = '\0'; // Necesario para guardar lo último escrito al dispositivo. Identifica el final del puntero de chars.

        msg_Ptr = msg;

        printk(KERN_ALERT "Mensaje guardado para ser leido por cat /dev/UNGS. \n");


        return i;

}

MODULE_LICENSE("GPL") ;
MODULE_AUTHOR("UNGS") ;
MODULE_DESCRIPTION("Driver - Sabrina Saczkowski");
