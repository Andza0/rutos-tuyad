include $(TOPDIR)/rules.mk

PKG_NAME:=tuyad
PKG_RELEASE=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

ifdef CONFIG_USE_UCLIBC
CONFIGURE_VARS += \
    LIBS="-largp"
endif

define Package/tuyad
	CATEGORY:=Base system
	TITLE:=tuyad
	DEPENDS:=+tuya-iot-core-sdk +USE_UCLIBC:argp-standalone +cJSON +libubus +libubox +libblobmsg-json
endef

define Package/tuyad/description
	System monitoring service
endef

define Package/tuyad/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuyad $(1)/usr/bin
	$(INSTALL_BIN) ./files/tuyad.init $(1)/etc/init.d/tuyad
	$(INSTALL_CONF) ./files/tuyad.config $(1)/etc/config/tuyad
endef

$(eval $(call BuildPackage,tuyad))
