#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0x4f3bf785, "alloc_pages_current" },
	{ 0x42e80c19, "cdev_del" },
	{ 0x4f1939c7, "per_cpu__current_task" },
	{ 0xc917223d, "pci_bus_read_config_byte" },
	{ 0xc45a9f63, "cdev_init" },
	{ 0xd2037915, "dev_set_drvdata" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0xdd822018, "boot_cpu_data" },
	{ 0xa30682, "pci_disable_device" },
	{ 0x799c50a, "param_set_ulong" },
	{ 0x973873ab, "_spin_lock" },
	{ 0xfc4f55f3, "down_interruptible" },
	{ 0x7edc1537, "device_destroy" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x102b9c3, "pci_release_regions" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x1f31615f, "pci_bus_write_config_word" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x343a1a8, "__list_add" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xb8e7ce2c, "__put_user_8" },
	{ 0x747f9a8e, "pci_iounmap" },
	{ 0xea147363, "printk" },
	{ 0xacdeb154, "__tracepoint_module_get" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x85f8a266, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0xae290fb6, "pci_bus_write_config_dword" },
	{ 0x521445b, "list_del" },
	{ 0x2d2cf7d, "device_create" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0xa6d1bdca, "cdev_add" },
	{ 0xcbd81171, "module_put" },
	{ 0xffd35acd, "__free_pages" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0x84b453e6, "pci_bus_read_config_word" },
	{ 0xc5aa6d66, "pci_bus_read_config_dword" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x68f7c535, "pci_unregister_driver" },
	{ 0x91766c09, "param_get_ulong" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0x32047ad5, "__per_cpu_offset" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0x4d7d27b8, "pci_bus_write_config_byte" },
	{ 0x8c183cbe, "iowrite16" },
	{ 0x37a0cba, "kfree" },
	{ 0xc911f7f0, "remap_pfn_range" },
	{ 0x6d090f30, "pci_request_regions" },
	{ 0x3f1899f1, "up" },
	{ 0x5f07b9f3, "__pci_register_driver" },
	{ 0xe06bb002, "class_destroy" },
	{ 0xc5534d64, "ioread16" },
	{ 0x74ae34c9, "pci_iomap" },
	{ 0x436c2179, "iowrite32" },
	{ 0xa12add91, "pci_enable_device" },
	{ 0xa2654165, "__class_create" },
	{ 0x3302b500, "copy_from_user" },
	{ 0xa92a43c, "dev_get_drvdata" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xe484e35f, "ioread32" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00001057d00001801sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "E12B1B3396DEE38963E6F5A");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 9,
	.rhel_release = 695,
};
