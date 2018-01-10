#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0x36d56703, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xc534420e, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0x33abda09, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xffce31d0, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x8c719f3c, __VMLINUX_SYMBOL_STR(pci_irq_vector) },
	{ 0xe47c2c37, __VMLINUX_SYMBOL_STR(__dynamic_dev_dbg) },
	{ 0x73d0b6f9, __VMLINUX_SYMBOL_STR(dev_notice) },
	{ 0x7a2b175d, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x78764f4e, __VMLINUX_SYMBOL_STR(pv_irq_ops) },
	{ 0x3e7097a8, __VMLINUX_SYMBOL_STR(arch_dma_alloc_attrs) },
	{ 0x446c9964, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0xec6122ef, __VMLINUX_SYMBOL_STR(__uio_register_device) },
	{ 0xdc1fee85, __VMLINUX_SYMBOL_STR(sysfs_create_group) },
	{ 0x63cf0ffd, __VMLINUX_SYMBOL_STR(pci_intx_mask_supported) },
	{ 0x58ab11b5, __VMLINUX_SYMBOL_STR(pci_alloc_irq_vectors) },
	{ 0xa11b55b2, __VMLINUX_SYMBOL_STR(xen_start_info) },
	{ 0x731dba7a, __VMLINUX_SYMBOL_STR(xen_domain_type) },
	{ 0xa6d2c3d, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x9a3c4aae, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xdfa764a6, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x3a63733a, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xcfd40f48, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x480915dd, __VMLINUX_SYMBOL_STR(pci_check_and_mask_intx) },
	{ 0x7d0d43f5, __VMLINUX_SYMBOL_STR(pci_intx) },
	{ 0xbbd9b1c6, __VMLINUX_SYMBOL_STR(pci_cfg_access_unlock) },
	{ 0xed943bc2, __VMLINUX_SYMBOL_STR(pci_cfg_access_lock) },
	{ 0xccf6c1fb, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x6b0ae6e3, __VMLINUX_SYMBOL_STR(pci_reset_function) },
	{ 0x259572c0, __VMLINUX_SYMBOL_STR(pci_clear_master) },
	{ 0xa4e66a49, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x5944d015, __VMLINUX_SYMBOL_STR(__cachemode2pte_tbl) },
	{ 0x80acdda4, __VMLINUX_SYMBOL_STR(boot_cpu_data) },
	{ 0xdd25efee, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xcb5f9628, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x5d8251c, __VMLINUX_SYMBOL_STR(uio_unregister_device) },
	{ 0x72f999e1, __VMLINUX_SYMBOL_STR(sysfs_remove_group) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x8cb74290, __VMLINUX_SYMBOL_STR(pci_bus_type) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x562795ed, __VMLINUX_SYMBOL_STR(pci_enable_sriov) },
	{ 0xcc82810d, __VMLINUX_SYMBOL_STR(pci_disable_sriov) },
	{ 0xf2db0247, __VMLINUX_SYMBOL_STR(pci_num_vf) },
	{ 0x60ea2d6, __VMLINUX_SYMBOL_STR(kstrtoull) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=uio";

