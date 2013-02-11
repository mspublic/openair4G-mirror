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
	{ 0xc709c132, "module_layout" },
	{ 0x4e3c1d16, "kmalloc_caches" },
	{ 0x2b200864, "_raw_spin_unlock" },
	{ 0x1574c0b9, "del_timer" },
	{ 0x9e1bdc28, "init_timer_key" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xf2663196, "netif_rx" },
	{ 0x850ff1d8, "netlink_kernel_create" },
	{ 0x2bc95bd4, "memset" },
	{ 0x7de5f22b, "dev_alloc_skb" },
	{ 0x50eedeb8, "printk" },
	{ 0x75b5c709, "netlink_kernel_release" },
	{ 0xf15ecb2d, "free_netdev" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x7bcadfd4, "register_netdev" },
	{ 0xb4390f9a, "mcount" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x84c554c7, "netlink_unicast" },
	{ 0x71205378, "add_timer" },
	{ 0xca5cb963, "init_net" },
	{ 0xcc9f64df, "__alloc_skb" },
	{ 0x3b1a706b, "alloc_netdev_mqs" },
	{ 0xd6a0919d, "eth_type_trans" },
	{ 0xa1d295c6, "ether_setup" },
	{ 0xd4eb00f, "kmem_cache_alloc_trace" },
	{ 0x6443d74d, "_raw_spin_lock" },
	{ 0xd3b84c9e, "param_ops_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0xd1329880, "param_array_ops" },
	{ 0x80f90992, "unregister_netdev" },
	{ 0x1824f77b, "__netif_schedule" },
	{ 0x3fa8adde, "consume_skb" },
	{ 0xaafc4535, "skb_put" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x13095525, "param_ops_uint" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "67F7A7DD7EC1D0D4D847070");
