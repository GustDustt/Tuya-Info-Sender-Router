include $(TOPDIR)/rules.mk

PKG_NAME:=vuci-app-tuya-daemon-api
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/vuci-app-tuya-daemon-api
  SECTION:=vuci
  CATEGORY:=VuCI
  SUBMENU:=Applications
  TITLE:=Tuya Daemon API Endpoint
  DEPENDS:=+rpcd +libubus-lua
  PKG_TLT_NAME:=Tuya Daemon API
endef

define Package/vuci-app-tuya-daemon-api/description
  Backend API endpoint for controlling the tuya-daemon service.
endef

define Build/Compile
endef

define Package/vuci-app-tuya-daemon-api/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/acl.d/tuya.json $(1)/usr/share/rpcd/acl.d/

	$(INSTALL_DIR) $(1)/usr/share/vuci/path.d
	$(INSTALL_DATA) ./files/usr/share/vuci/path.d/tuya.json $(1)/usr/share/vuci/path.d/

	$(INSTALL_DIR) $(1)/usr/lib/lua/api/services
	$(INSTALL_DATA) ./files/usr/lib/lua/api/services/tuya.lua $(1)/usr/lib/lua/api/services/
endef

$(eval $(call BuildPackage,vuci-app-tuya-daemon-api))