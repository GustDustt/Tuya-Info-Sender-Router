include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-daemon
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKGARCH:=all
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tuya-daemon
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tuya System Monitor Daemon
  DEPENDS:=+libubus +libubox +libblobmsg-json +libpthread +libopenssl
endef

define Package/tuya-daemon/description
  A daemon that gathers system metrics over ubus and reports to Tuya IoT Cloud.
endef

define Build/Install
endef

define Package/tuya-daemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sys_monitor_daemon $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tuya-daemon))