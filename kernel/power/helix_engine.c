/* linux/kernel/power/helix_engine.c
 *
 * Copyright (C) 2017 ZeroInfinity
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 
#include <linux/module.h>

#include "power.h"


static struct kobject *helix_engine_kobj;
static struct kobject *app_engine_kobj;
static struct kobject *thermal_manager_kobj;
static struct kobject *suspend_engine_kobj;

#define define_string_show(_name, str_buf)				\
static ssize_t _name##_show						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)		\
{									\
	return scnprintf(buf, sizeof(str_buf), "%s\n", str_buf);	\
}

#define define_string_store(_name, str_buf, store_cb)		\
static ssize_t _name##_store					\
(struct kobject *kobj, struct kobj_attribute *attr,		\
 const char *buf, size_t n)					\
{								\
	strncpy(str_buf, buf, sizeof(str_buf) - 1);				\
	str_buf[sizeof(str_buf) - 1] = '\0';				\
	(store_cb)(#_name);					\
	sysfs_notify(kobj, NULL, #_name);			\
	return n;						\
}

#define define_int_show(_name, int_val)				\
static ssize_t _name##_show					\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)	\
{								\
	return sprintf(buf, "%d\n", int_val);			\
}

#define define_int_store(_name, int_val, store_cb)		\
static ssize_t _name##_store					\
(struct kobject *kobj, struct kobj_attribute *attr,		\
 const char *buf, size_t n)					\
{								\
	int val;						\
	if (sscanf(buf, "%d", &val) > 0) {			\
		int_val = val;					\
		(store_cb)(#_name);				\
		sysfs_notify(kobj, NULL, #_name);		\
		return n;					\
	}							\
	return -EINVAL;						\
}

static void null_cb(const char *attr) {
	do { } while (0);
}

static int mode_value = 0;
static int throttlecap_value = 0;
static int enginecap_value = 0;
static int per_app_use_value = 0;
static int enable_value = 1;

define_int_show(mode, mode_value);
define_int_store(mode, mode_value, null_cb);
power_attr(mode);

define_int_show(throttlecap, throttlecap_value);
define_int_store(throttlecap, throttlecap_value, null_cb);
power_attr(throttlecap);

define_int_show(enginecap, enginecap_value);
define_int_store(enginecap, enginecap_value, null_cb);
power_attr(enginecap);

define_int_show(per_app_use, per_app_use_value);
define_int_store(per_app_use, per_app_use_value, null_cb);
power_attr(per_app_use);

define_int_show(enable, enable_value);
define_int_store(enable, enable_value, null_cb);
power_attr(enable);

static struct attribute *helix_engine_g[] = {
	&enable_attr.attr,
	NULL,
};

static struct attribute *app_engine_g[] = {
	&per_app_use_attr.attr,
	&enable_attr.attr,
	&enginecap_attr.attr,
	NULL,
};

static struct attribute *thermal_manager_g[] = {
	&throttlecap_attr.attr,
	&enable_attr.attr,
	&mode_attr.attr,
	NULL,
};

static struct attribute *suspend_engine_g[] = {
	&enable_attr.attr,
	NULL,
};

static struct attribute_group helix_engine_attr_group = {
	.attrs = helix_engine_g,
};

static struct attribute_group app_engine_attr_group = {
	.attrs = app_engine_g,
};

static struct attribute_group thermal_manager_attr_group = {
	.attrs = thermal_manager_g,
};

static struct attribute_group suspend_engine_attr_group = {
	.attrs = suspend_engine_g,
};

static int __init helix_engine_init(void)
{

	int ret = 0;

	helix_engine_kobj = kobject_create_and_add("helix_engine", power_kobj);

	if (!helix_engine_kobj) {
		pr_err("%s: Can not allocate enough memory for helix engine.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	
	app_engine_kobj = kobject_create_and_add("app_engine", helix_engine_kobj);
	thermal_manager_kobj = kobject_create_and_add("thermal_manager", helix_engine_kobj);
	suspend_engine_kobj = kobject_create_and_add("suspend_engine", helix_engine_kobj);

	if (!app_engine_kobj || !thermal_manager_kobj || !suspend_engine_kobj) {
		pr_err("%s: Can not allocate enough memory.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * Create all attribute group under each node.
	 */
	ret = sysfs_create_group(helix_engine_kobj, &helix_engine_attr_group);
	ret |= sysfs_create_group(app_engine_kobj, &app_engine_attr_group);
	ret |= sysfs_create_group(thermal_manager_kobj, &thermal_manager_attr_group);
	ret |= sysfs_create_group(suspend_engine_kobj, &suspend_engine_attr_group);

	if (ret) {
		pr_err("%s: sysfs_create_group failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	return 0;

err:
	pr_err("helix engine init failed\n");
	return ret;
}

static void  __exit helix_engine_exit(void)
{

	sysfs_remove_group(helix_engine_kobj, &helix_engine_attr_group);
	sysfs_remove_group(app_engine_kobj, &app_engine_attr_group);
	sysfs_remove_group(thermal_manager_kobj, &thermal_manager_attr_group);
	sysfs_remove_group(suspend_engine_kobj, &suspend_engine_attr_group);
}

module_init(helix_engine_init);
module_exit(helix_engine_exit);
