product-support-types+=dblog
product-def-all-items+=dblog.modules

dblog-tool=$(CPDE_ROOT)/buildtools/tools/gen-dblog.pl

define product-def-rule-dblog-module

$(call assert-not-null,$1.dblog.$3.source)
$(call assert-not-null,$1.dblog.$3.output-h)
$(call assert-not-null,$1.dblog.$3.output-c)
$(call assert-not-null,$1.dblog.$3.output-bin)
$(call assert-not-null,$1.dblog.$3.prefix)

$(eval r.$1.$2.dblog.$3.source:=$($1.dblog.$3.source))
$(eval r.$1.$2.dblog.$3.output-h:=$($1.dblog.$3.output-h))
$(eval r.$1.$2.dblog.$3.output-c:=$($1.dblog.$3.output-c))
$(eval r.$1.$2.dblog.$3.prefix:=$($1.dblog.$3.prefix))
$(eval r.$1.$2.dblog.$3.generated.h:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$($1.dblog.$3.output-h),$2))
$(eval r.$1.$2.dblog.$3.generated.c:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$($1.dblog.$3.output-c),$2))
$(eval r.$1.$2.dblog.$3.generated.meta:=$(dir $(r.$1.$2.dblog.$3.generated.c))/meta)
$(eval r.$1.$2.dblog.$3.generated.meta.c:=$(r.$1.$2.dblog.$3.generated.meta)/$(notdir $(r.$1.$2.dblog.$3.generated.c)).lib.c)
$(eval r.$1.$2.dblog.$3.generated.meta.bin:=$(CPDE_OUTPUT_ROOT)/$($2.output)/$($1.dblog.$3.output-bin))

$(eval r.$1.$2.c.sources += $(r.$1.$2.dblog.$3.generated.c) $(r.$1.$2.dblog.$3.generated.h) $(r.$1.$2.dblog.$3.generated.meta.c))
$(eval r.$1.$3.generated-sources += $(r.$1.$2.dblog.$3.generated.c) $(r.$1.$2.dblog.$3.generated.h) $(r.$1.$2.dblog.$3.generated.meta.c))
$(eval r.$1.cleanup += $(r.$1.$2.dblog.$3.generated.c) \
                       $(r.$1.$2.dblog.$3.generated.h) \
                       $(r.$1.$2.dblog.$3.generated.meta.c) \
                       $(r.$1.$2.dblog.$3.generated.meta.bin) )

auto-build-dirs += $(dir $(r.$1.$2.dblog.$3.generated.c) \
                         $(r.$1.$2.dblog.$3.generated.h) \
                         $(r.$1.$2.dblog.$3.generated.meta.bin)) \
                   $(r.$1.$2.dblog.$3.generated.meta)

$2.$1: $(r.$1.$2.dblog.$3.generated.meta.bin)

$(call c-source-to-object,$(r.$1.c.sources),$2): $(r.$1.$2.dblog.$3.generated.h)

$(r.$1.$2.dblog.$3.generated.meta.bin) $(r.$1.$2.dblog.$3.generated.meta.c): $$(r.$1.$2.dblog.$3.source) $(cpe-dr-tool)
	$$(call with_message,dblog metalib generaing to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.dblog.$2.generated.c)) ...) \
	$(cpe-dr-tool) -i $(r.$1.$2.dblog.$3.source) \
                   --output-h $(r.$1.$2.dblog.$3.generated.meta) \
                   --output-lib-c $(r.$1.$2.dblog.$3.generated.meta.c) \
                   --output-lib-c-arg g_metalib_$(1)_oa \
                   --output-lib-bin $(r.$1.$2.dblog.$3.generated.meta.bin)

$(r.$1.$2.dblog.$3.generated.c) $(r.$1.$2.dblog.$3.generated.h): $$(r.$1.$2.dblog.$3.source) $(dblog-tool) $(r.$1.$2.dblog.$3.generated.meta.c)
	$$(call with_message,dblog itf generaing to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.dblog.$2.generated.c)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(CPDE_PERL) $(dblog-tool) --input-file $(r.$1.$2.dblog.$3.source) \
                       --output-c $(r.$1.$2.dblog.$3.generated.c) \
                       --output-h $(r.$1.$2.dblog.$3.generated.h) \
                       --prefix $(r.$1.$2.dblog.$3.prefix) \
                       --metalib g_metalib_$(1)_oa \
                       $(addprefix --meta-h meta/, $(patsubst %.xml,%.h,$(notdir $(r.$1.$2.dblog.$3.source))))

endef

define product-def-rule-dblog

$(foreach module,$(r.$1.dblog.modules),\
	$(call product-def-rule-dblog-module,$1,$2,$(module)))

endef
