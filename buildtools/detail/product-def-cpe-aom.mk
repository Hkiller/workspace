product-support-types+=cpe-aom
product-def-all-items+=cpe-aom.modules

cpe-aom-tool=$$(CPDE_OUTPUT_ROOT)/$$(tools.output)/bin/cpe_aom_tool

define product-def-rule-cpe-aom-c-module-hpp
  $(call assert-not-null,$1.cpe-aom.$2.hpp.output)
  $(call assert-not-null,$1.cpe-aom.$2.hpp.class-name)

  $(eval r.$1.$3.cpe-aom.$2.hpp.output:=$($1.cpe-aom.$2.hpp.output))
  $(eval r.$1.$3.cpe-aom.$2.hpp.class-name:=$($1.cpe-aom.$2.hpp.class-name))
  $(eval r.$1.$3.cpe-aom.$2.hpp.namespace:=$($1.cpe-aom.$2.hpp.namespace))
  $(eval r.$1.$3.cpe-aom.$2.hpp.output-dir:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$(patsubst %/,%,$(dir $(r.$1.$3.cpe-aom.$2.hpp.output))),$3))
  $(eval r.$1.$3.cpe-aom.$2.generated.hpp:=$(r.$1.$3.cpe-aom.$2.hpp.output-dir)/$(notdir $($1.cpe-aom.$2.hpp.output)))

  $(call c-source-to-object,$(r.$1.c.sources),$3): $(r.$1.$3.cpe-aom.$2.generated.hpp)

  auto-build-dirs += $(r.$1.$3.cpe-aom.$2.hpp.output-dir)

  $(r.$1.$3.cpe-aom.$2.generated.hpp): $(r.$1.$3.cpe-aom.$2.aom-meta-source) $(r.$1.$3.cpe-aom.$2.dr-meta-source) $(cpe-aom-tool)
	$$(call with_message,cpe-aom generaing hpp to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.cpe-aom.$2.generated.hpp)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(cpe-aom-tool) mk-hpp \
                    $(addprefix --from-aom-meta , $(r.$1.$3.cpe-aom.$2.aom-meta-source)) \
                    $(addprefix --from-dr-name , $(r.$1.$3.cpe-aom.$2.aom-dr-name)) \
                    $(addprefix --dr-meta , $(r.$1.$3.cpe-aom.$2.dr-meta-source)) \
                    $(addprefix --align , $(r.$1.$3.cpe-aom.$2.align)) \
                    $(addprefix --validate-, $(r.$1.$3.cpe-aom.$2.validate)) \
                    --output-hpp $$@ \
                    --class-name $($1.cpe-aom.$2.hpp.class-name) \
                    $(addprefix --namespace ,$($1.cpe-aom.$2.hpp.namespace))

endef

define product-def-rule-cpe-aom-c-module
$(call assert-not-null,$1.cpe-aom.$3.dr-meta-source)
$(call assert-not-null,$1.cpe-aom.$3.generate)

$(eval r.$1.$2.cpe-aom.$3.aom-dr-name:=$($1.cpe-aom.$3.aom-dr-name))
$(eval r.$1.$2.cpe-aom.$3.dr-meta-source:=$($1.cpe-aom.$3.dr-meta-source))
$(eval r.$1.$2.cpe-aom.$3.generate:=$($1.cpe-aom.$3.generate))
$(eval r.$1.$2.cpe-aom.$3.align:=$($1.cpe-aom.$3.align))
$(eval r.$1.$2.cpe-aom.$3.validate:=$($1.cpe-aom.$3.validate))

$(foreach p,$(r.$1.$2.cpe-aom.$3.generate), $(call product-def-rule-cpe-aom-c-module-$p,$1,$3,$2))

endef

define product-def-rule-cpe-aom

$(foreach module,$(r.$1.cpe-aom.modules),\
	$(call product-def-rule-cpe-aom-c-module,$1,$2,$(module)))

endef
