
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/module.h>

//Socket Header Files
#include <linux/socket.h>
#include <linux/net.h>
#include <net/sock.h>
#include "ksocket.h"

static int port = 9988;


struct input_dev *cloudaccl_input_dev;        /* Representation of an input device */
static struct platform_device *cloudaccl_dev; /* Device structure */
static int enable_update;

/* Sysfs method */
static ssize_t read_enable(struct device *dev,
          struct device_attribute *attr,
          char *buffer)
{
   
    printk ("read_enable callled\n");
    return sprintf(buffer, "%u\n",enable_update);
}



static ssize_t write_enable(struct device *dev,
          struct device_attribute *attr,
          const char *buffer, size_t count)
{
    int en,x,y;
    x=0;
    y=1;
    
    sscanf(buffer, "%d", &en);
    printk ("write_enable callled with prior enable_update:%d\n", enable_update);
    enable_update = en;
    /*while(enable_update!=0){
    	
    }*/
    
    printk ("write_enable callled with en:%d, enable_update:%d\n", en, enable_update);
    /* Report relative ax via the
       event interface */
    input_report_rel(cloudaccl_input_dev, REL_X, x);
    input_report_rel(cloudaccl_input_dev, REL_Y, y);
    input_sync(cloudaccl_input_dev);
    
    printk(KERN_ALERT "Creating Socket now\n");
    ksocket_t sockfd_cli;
    struct sockaddr_in addr_srv;
    char buf[1024], *tmp;
    int addr_len,len;

#ifdef KSOCKET_ADDR_SAFE
	mm_segment_t old_fs;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	//sprintf(current->comm, "ksktcli");	
	//*buf = "Hello!";
	memset(&addr_srv, 0, sizeof(addr_srv));
	addr_srv.sin_family = AF_INET;
	addr_srv.sin_port = htons(port);
	addr_srv.sin_addr.s_addr = inet_addr("127.0.0.1");;
	addr_len = sizeof(struct sockaddr_in);
	
	sockfd_cli = ksocket(AF_INET, SOCK_STREAM, 0);
	printk("sockfd_cli = 0x%p\n", sockfd_cli);
	if (sockfd_cli == NULL)
	{
		printk("socket failed\n");
		return -1;
	}
	if (kconnect(sockfd_cli, (struct sockaddr*)&addr_srv, addr_len) < 0)
	{
		printk("connect failed\n");
		return -1;
	}

	tmp = inet_ntoa(addr_srv.sin_addr);
	printk("connected to : %s %d\n", tmp, ntohs(addr_srv.sin_port));
	kfree(tmp);
	
	len = sprintf(buf, "%s", "This is accl\n");
    ksend(sockfd_cli, buf, len, 0);
	
	krecv(sockfd_cli, buf, 1024, 0);
	printk("got message : %s\n", buf);

	
	
	printk(KERN_ALERT "Closing dell led socket now!\n");
	kclose(sockfd_cli);
#ifdef KSOCKET_ADDR_SAFE
	set_fs(old_fs);
#endif
    
    
    
    return 0;
}

/* Attach the sysfs write method */
DEVICE_ATTR(ax, 0644, read_enable, write_enable);

/* Attribute Descriptor */
static struct attribute *cloudaccl_attrs[] = {
    &dev_attr_ax.attr,
    NULL
};

/* Attribute group */
static struct attribute_group cloudaccl_attr_group = {
    .attrs = cloudaccl_attrs,
};

/* Driver Initialization */
int __init
cloudaccl_init(void)
{
    /* Register a platform device */
    cloudaccl_dev = platform_device_register_simple("cloudaccl", -1, NULL, 0);
    if (IS_ERR(cloudaccl_dev)){
        printk ("cloudaccl_init: error\n");
        return PTR_ERR(cloudaccl_dev);
    }

    /* Create a sysfs node to read simulated ax */
    sysfs_create_group(&cloudaccl_dev->dev.kobj, &cloudaccl_attr_group);

    /* Allocate an input device data structure */
    cloudaccl_input_dev = input_allocate_device();
    if (!cloudaccl_input_dev) {
        printk("Bad input_allocate_device()\n"); return -ENOMEM;
    }

    /* Announce that the virtual mouse will generate
       relative ax */
    set_bit(EV_REL, cloudaccl_input_dev->evbit);
    set_bit(REL_X, cloudaccl_input_dev->relbit);
    set_bit(REL_Y, cloudaccl_input_dev->relbit);

    /* Register with the input subsystem */
    input_register_device(cloudaccl_input_dev);

    printk("Cloud Accelerometer Driver Initialized.\n");
    return 0;
}

/* Driver Exit */
void
cloudaccl_cleanup(void)
{
    /* Unregister from the input subsystem */
    input_unregister_device(cloudaccl_input_dev);

    /* Cleanup sysfs node */
    sysfs_remove_group(&cloudaccl_dev->dev.kobj, &cloudaccl_attr_group);

    /* Unregister driver */
    platform_device_unregister(cloudaccl_dev);
    printk("Cloud Accelerometer Driver Removed.\n");
    return;
}

module_init(cloudaccl_init);
module_exit(cloudaccl_cleanup);

MODULE_AUTHOR("Arpit");
MODULE_DESCRIPTION("Cloud supported Accelerometer Driver");
MODULE_LICENSE("Dual BSD/GPL");


