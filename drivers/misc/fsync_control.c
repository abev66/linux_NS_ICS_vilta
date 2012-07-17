/* drivers/misc/fsync_control.c
 *
 * Copyright 2012  Ezekeel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#define FSYNCCONTROL_VERSION 1

static bool fsync_enabled = false;
static unsigned int do_fsync_counter = 1000;

bool fsynccontrol_fsync_enabled()
{
    static unsigned int skip_fsync_count = 0;
  
    if(!fsync_enabled && (do_fsync_counter != 0))
      if(skip_fsync_count > do_fsync_counter) {
	skip_fsync_count = 0;
	pr_info("fsync_control: do fsync\n");
	return true;
      }
      else
	skip_fsync_count++;
      
    return fsync_enabled;
}
EXPORT_SYMBOL(fsynccontrol_fsync_enabled);

static ssize_t fsynccontrol_status_read(struct device * dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n", (fsync_enabled ? 1 : 0));
}

static ssize_t fsynccontrol_status_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    unsigned int data;

    if(sscanf(buf, "%u\n", &data) == 1) 
	{
	    if (data == 1) 
		{
		    pr_info("%s: FSYNCCONTROL fsync enabled\n", __FUNCTION__);

		    fsync_enabled = true;

		} 
	    else if (data == 0) 
		{
		    pr_info("%s: FSYNCCONTROL fsync disabled\n", __FUNCTION__);

		    fsync_enabled = false;
		} 
	    else 
		{
		    pr_info("%s: invalid input range %u\n", __FUNCTION__, data);
		}
	} 
    else 
	{
	    pr_info("%s: invalid input\n", __FUNCTION__);
	}

    return size;
}

static ssize_t fsynccontrol_version(struct device * dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n", FSYNCCONTROL_VERSION);
}

static ssize_t do_fsync_counter_status_read(struct device * dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n", do_fsync_counter);
}

static ssize_t do_fsync_counter_status_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    unsigned int data;
    
    if(sscanf(buf, "%u\n", &data) == 1) {
	do_fsync_counter = data;
	pr_info("%s: do_fsync_counter = %u\n", __FUNCTION__, do_fsync_counter);
    } else
	pr_info("%s: invalid input\n", __FUNCTION__);
    
    return size;
}

static DEVICE_ATTR(fsync_enabled, S_IRUGO | S_IWUGO, fsynccontrol_status_read, fsynccontrol_status_write);
static DEVICE_ATTR(version, S_IRUGO , fsynccontrol_version, NULL);
static DEVICE_ATTR(do_fsync_counter, S_IRUGO | S_IWUGO , do_fsync_counter_status_read, do_fsync_counter_status_write);

static struct attribute *fsynccontrol_attributes[] = 
    {
	&dev_attr_fsync_enabled.attr,
	&dev_attr_version.attr,
	&dev_attr_do_fsync_counter.attr,
	NULL
    };

static struct attribute_group fsynccontrol_group = 
    {
	.attrs  = fsynccontrol_attributes,
    };

static struct miscdevice fsynccontrol_device = 
    {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fsynccontrol",
    };
    
static struct miscdevice do_fsync_counter_device = 
    {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "do_fsync_counter",
    };

static int __init fsynccontrol_init(void)
{
    int ret;

    pr_info("%s misc_register(%s)\n", __FUNCTION__, fsynccontrol_device.name);

    ret = misc_register(&fsynccontrol_device);

    if (ret) 
	{
	    pr_err("%s misc_register(%s) fail\n", __FUNCTION__, fsynccontrol_device.name);
	    return 1;
	}

    if (sysfs_create_group(&fsynccontrol_device.this_device->kobj, &fsynccontrol_group) < 0) 
	{
	    pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
	    pr_err("Failed to create sysfs group for device (%s)!\n", fsynccontrol_device.name);
	}

    return 0;
}

device_initcall(fsynccontrol_init);
