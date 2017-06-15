
PKG_FILE=$(PROJECT_BASE)pci_install.tar
PKG_INSTALL_DIR=$(subst //,/,$(INSTALL_DIRECTORY))
PKG_TAR_DIR=$(subst //,/,$(subst $(CPUVARDIR),,$(subst $(INSTALLDIR),,$(PKG_INSTALL_DIR))))
PKG_TAR_FILE_DIR=$(subst $(PKG_TAR_DIR),,$(PKG_INSTALL_DIR))

package: install
	@for pkg_file in $(PACKAGE_FILES); do \
		cd $(PKG_TAR_DIR); \
		tar --preserve-permissions -uvf $(PKG_FILE) $(PKG_TAR_FILE_DIR)$$pkg_file; \
	done; \
	cd $(PKG_TAR_DIR); \
	tar --preserve-permissions -uvf $(PKG_FILE) usr/include/pci
