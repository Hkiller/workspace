product-support-types+=run
product-def-all-items+=run.libraries run.cmd run.path run.args

define product-def-rule-run

$(eval r.$1.$2.run.cmd:=$(if $(r.$1.$2.run.cmd),$(r.$1.$2.run.cmd),$(r.$1.run.cmd)))

$(eval r.$1.$2.run.full-cmd:= \
		$$(if $$(r.$1.$2.run.path),cd $$(r.$1.$2.run.path) &&,$$(if $$(r.$1.run.path),cd $$(r.$1.run.path) &&,)) \
		$$(if $$(r.$1.run.libraries)$$(r.$1.$2.run.libraries),LD_LIBRARY_PATH=$$(call path-list-join,$$(r.$1.run.libraries)$$(r.$1.$2.run.libraries))$$$$LD_LIBRARY_PATH ,) \
		$(r.$1.$2.run.cmd) \
		$$(if $$(r.$1.$2.run.args),$$(r.$1.$2.run.args),$$(r.$1.run.args)))

.PHONY: $2.$1.run

$(if $(filter $(CPDE_OUTPUT_ROOT)/$(tools.output)/%,$(r.$1.$2.run.cmd)),$2.$1: $(r.$1.$2.run.cmd))

$2.$1.run: $2.$1
	$(call with_message,runing $1 ...)\
		$$(r.$1.$2.run.full-cmd)

endef
