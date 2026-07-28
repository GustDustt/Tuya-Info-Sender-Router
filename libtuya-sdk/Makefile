include $(TOPDIR)/rules.mk

PKG_NAME:=libtuya-sdk
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tuya/tuya-iot-core-sdk.git
PKG_SOURCE_VERSION:=main
PKG_MIRROR_HASH:=skip

CMAKE_BINARY_SUBDIR:=build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

PKG_INSTALL:=0

define Package/libtuya-sdk
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Tuya IoT Core C SDK
  DEPENDS:=+libpthread +librt +libopenssl
endef

define Package/libtuya-sdk/description
  Tuya IoT Core SDK packaged for RutOS.
endef

CMAKE_OPTIONS += \
    -DBUILD_SHARED_LIBS=OFF \
    -DENABLE_TESTS=OFF

define Build/Install/Default
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/tuya
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/tuya/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/src/*.h $(1)/usr/include/tuya/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/utils/*.h $(1)/usr/include/tuya/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/middleware/*.h $(1)/usr/include/tuya/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/platform/posix/*.h $(1)/usr/include/tuya/ 2>/dev/null || true
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/build/lib/*.a $(1)/usr/lib/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/build/*.a $(1)/usr/lib/ 2>/dev/null || true
endef

define Package/libtuya-sdk/install
	$(INSTALL_DIR) $(1)/usr/lib
endef

$(eval $(call BuildPackage,libtuya-sdk))