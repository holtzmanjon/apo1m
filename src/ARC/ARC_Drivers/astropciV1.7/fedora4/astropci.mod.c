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
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x89e24b9c, "struct_module" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xa03d6a57, "__get_user_4" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xb5f46a8b, "pci_bus_write_config_byte" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x26e96637, "request_irq" },
	{ 0x3762cb6e, "ioremap_nocache" },
	{ 0x3a626247, "pci_request_region" },
	{ 0xdd994dbd, "pci_enable_device" },
	{ 0x40e4fec5, "wake_up_process" },
	{ 0x4086729e, "register_chrdev" },
	{ 0x13f3405e, "__pci_register_driver" },
	{ 0x2b6754b3, "remap_pfn_range" },
	{ 0xc4d65887, "boot_cpu_data" },
	{ 0x8a7d1c31, "high_memory" },
	{ 0x1bcd461f, "_spin_lock" },
	{ 0x2e60bace, "memcpy" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0x3ffa5ef4, "pci_disable_device" },
	{ 0xb0421b76, "pci_release_regions" },
	{ 0xedc03953, "iounmap" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0x7423476b, "pci_unregister_driver" },
	{ 0x1b7d4074, "printk" },
	{ 0xc192d491, "unregister_chrdev" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00001057d00001801sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "EA84928F463FA3E293A4297");
