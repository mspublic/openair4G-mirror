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
	{ 0xad12b0d5, "module_layout" },
	{ 0xfac3858a, "kmalloc_caches" },
	{ 0xb279da12, "pv_lock_ops" },
	{ 0x6307fc98, "del_timer" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x43ab66c3, "param_array_get" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x45947727, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0x2fbd3bb, "netif_rx" },
	{ 0x3d6d0930, "netlink_kernel_create" },
	{ 0x9d8e6569, "dev_alloc_skb" },
	{ 0xb72397d5, "printk" },
	{ 0x3238cab4, "netlink_kernel_release" },
	{ 0x1e2669d7, "free_netdev" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xa7591d5e, "register_netdev" },
	{ 0xb4390f9a, "mcount" },
	{ 0x8abdf654, "netlink_unicast" },
	{ 0x744c0c68, "param_get_byte" },
	{ 0x46085e4f, "add_timer" },
	{ 0xe1048212, "init_net" },
	{ 0xdeabe977, "kmem_cache_alloc" },
	{ 0x91aa4280, "__alloc_skb" },
	{ 0x108e8985, "param_get_uint" },
	{ 0xa9b4fdb, "alloc_netdev_mq" },
	{ 0x72c3be87, "param_set_byte" },
	{ 0x9c19ad20, "eth_type_trans" },
	{ 0x18f5418e, "ether_setup" },
	{ 0x37a0cba, "kfree" },
	{ 0x3285cc48, "param_set_uint" },
	{ 0x1f07d4a6, "unregister_netdev" },
	{ 0x14423bf6, "__netif_schedule" },
	{ 0xf3149e88, "consume_skb" },
	{ 0x3e7d80ba, "skb_put" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "846A0636EA82C07F2F62100");
