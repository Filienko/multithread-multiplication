#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf704969, "module_layout" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xab0324d4, "init_task" },
	{ 0x92997ed8, "_printk" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x4c55f5ef, "pv_ops" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0x8a35b432, "sme_me_mask" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xa92ec74, "boot_cpu_data" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "39B3F86A2AEFBF219C7CDF3");
