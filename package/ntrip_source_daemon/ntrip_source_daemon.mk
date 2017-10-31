################################################################################
#
# ntrip_source_daemon
#
################################################################################

NTRIP_SOURCE_DAEMON_VERSION = 0.1
NTRIP_SOURCE_DAEMON_SITE = \
  "${BR2_EXTERNAL_piksi_buildroot_PATH}/package/ntrip_source_daemon/src"
NTRIP_SOURCE_DAEMON_SITE_METHOD = local
NTRIP_SOURCE_DAEMON_DEPENDENCIES = czmq libsbp libpiksi libcurl libnetwork

define NTRIP_SOURCE_DAEMON_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) -C $(@D) all
endef

define NTRIP_SOURCE_DAEMON_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/ntrip_source_daemon $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
