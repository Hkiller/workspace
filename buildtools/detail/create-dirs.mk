
$(eval $(call debug-warning,auto-build-dirs=$(sort $(auto-build-dirs))))

force-create-dir:=$(foreach d,$(sort $(auto-build-dirs)),$(shell mkdir -p $d))
