product-support-types+=flex-as
product-def-all-items+=flex.as.sources flex.as.exports flex.libraries flex.sys-libraries flex.vfs flex.swig flex.size

#生成consol链接命令
define do-product-def-rule-flex-as-def-progn

$(eval r.$1.$2.flex-as.output:=$(CPDE_OUTPUT_ROOT)/$($2.output)/obj/$(subst $(CPDE_ROOT)/,,$(r.$1.base))/Console.abc)
$(eval r.$1.flex.product.c.flags.ld+=-symbol-abc=$(r.$1.$2.flex-as.output))

$(eval r.$1.$2.flex-symbols.output:=$(CPDE_OUTPUT_ROOT)/$($2.output)/obj/$(subst $(CPDE_ROOT)/,,$(r.$1.base))/symbols.c)
$(eval r.$1.$2.c.sources+=$(r.$1.$2.flex-symbols.output))

$(eval r.$1.$2.flex-exports.output:=$(CPDE_OUTPUT_ROOT)/$($2.output)/obj/$(subst $(CPDE_ROOT)/,,$(r.$1.base))/exports.txt)

$(eval r.$1.flex.product.c.flags.ld+=-swf-size=$$(r.$1.flex.size))

$(eval r.$1.flex.product.c.flags.ld+=$$(call product-gen-depend-value-list,$1,flex,flex.libraries) $$(r.$1.flex.libraries))
$(if $(filter $(flex),2),$(eval r.$1.flex.product.c.flags.ld+=-flto-api=$(r.$1.$2.flex-exports.output)))

auto-build-dirs += $(dir $(r.$1.$2.flex-as.output))

$2.$1: $$(r.$1.$2.flex-as.output)

$$(r.$1.$2.flex-as.output): $$(call product-gen-depend-value-list,$1,flex,flex.as.sources) \
                            $$(r.$1.flex.as.sources) \
                            $(call product-gen-depend-proj-list-with-type,$2,$1,flex-as)
	$$(flex.as3-comiler) \
		$$(foreach e,$$(call product-gen-depend-value-list,$1,flex,flex.sys-libraries) $$(r.$1.flex.sys-libraries), -import $$(call flex.regular-path,$$(FLASCC)/usr/lib/$$e)) \
		$$(foreach e,$$(call product-gen-depend-value-list,$1,flex,flex.libraries) $$(r.$1.flex.libraries), -import $$(call flex.regular-path,$$e)) \
		$$(foreach e,$$(filter-out %/HTTPBackingStore.as, $$(call product-gen-depend-value-list,$1,flex,flex.as.sources) $$(r.$1.flex.as.sources)), $$(call flex.regular-path,$$e)) \
		$$(foreach e,$$(r.$1.$2.as.addition-sources), $$(call flex.regular-path,$$e)) \
		-outdir $$(call flex.regular-path,$$(dir $$@)) -out $$(notdir $$(basename $$@))

$$(r.$1.$2.flex-exports.output): $$(call product-gen-depend-value-list,$1,flex,flex.as.exports)
	> $$@
	$$(foreach f,$$(call product-gen-depend-value-list,$1,flex,flex.as.exports),cat $$f >> $$@)

$$(CPDE_OUTPUT_ROOT)/$$(r.$1.$2.product): $$(r.$1.$2.flex-as.output) $$(r.$1.$2.flex-exports.output)

$$(r.$1.$2.flex-symbols.output): $$(r.$1.c.export-symbols) $$(call product-gen-depend-value-list,$1,$($2.env),c.export-symbols)
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-symbols.pl \
    --output $$@ \
    $$(addprefix --input ,$$(r.$1.c.export-symbols) $$(call product-gen-depend-value-list,$1,$($2.env),c.export-symbols)) \
    --compiler=$(call compiler-category,$(flex.CC))

endef

#生成swig链接命令
define do-product-def-rule-flex-as-def-swig

$(eval r.$1.$2.flex-swig.output:=$(CPDE_OUTPUT_ROOT)/$($2.output)/obj/$(subst $(CPDE_ROOT)/,,$(r.$1.base))/$1.cpp)
$(eval r.$1.$2.product.c.include:=$(dir $(r.$1.$2.flex-swig.output)))
$(eval r.$1.$2.c.sources:=$(r.$1.$2.flex-swig.output))
$(eval r.$1.product.c.output-includes+=.)
$(eval r.$1.flex.product.c.flags.ld+=-fllvm-llc-opt=-ascopt=-import \
			                         -fllvm-llc-opt=-ascopt=$$(call flex.regular-path,$$(r.$1.$2.flex-as.output)))

auto-build-dirs += $(dir $(r.$1.$2.flex-swig.output))

$2.$1: $$(r.$1.$2.flex-swig.output)

$$(r.$1.$2.flex-swig.output): $$(r.$1.$2.flex-as.output)
	$$(flex.swig) -i $$(call flex.regular-path,$$^) -o $$(call flex.regular-path,$$(basename $$@))

$(call c-source-to-object,$(r.$1.$2.flex-swig.output),$2): flex.CPPFLAGS+=-emit-llvm

$$(CPDE_OUTPUT_ROOT)/$$(r.$1.$2.product): $$(r.$1.$2.flex-swig.output)

endef

define do-product-def-rule-flex-as-def-vfs

$(eval r.$1.$2.flex.vfs.$3.output:=$($1.flex.vfs.output-dir)/$3.abc)
$(eval r.$1.flex.libraries+=$$(r.$1.$2.flex.vfs.$3.output) $$(FLASCC)/usr/lib/AlcVFSZip.abc)

$$(r.$1.$2.flex-as.output): $(r.$1.$2.flex.vfs.$3.output)

auto-build-dirs += $(if $(r.$1.$2.flex-swig.output),$(dir $(r.$1.$2.flex-swig.output))data)

$(r.$1.$2.flex.vfs.$3.output):
	$(if $($1.flex.vfs.$3.embed),$$(flex.vfs-generator) --name=$3 $$($1.flex.vfs.$3.embed) $$($1.flex.vfs.output-dir)/vfs_$3)
	$(if $($1.flex.vfs.$3.http),cd $$(dir $(CPDE_OUTPUT_ROOT)/$$(r.$1.$2.product)) \
                                && mkdir -p data \
                                && $$(flex.vfs-generator) --type=http --name=$3 $($1.flex.vfs.$3.http) data/vfs_$3 \
                                && mv data/vfs_$3_manifest.as $$($1.flex.vfs.output-dir)/manifest.as)
	$(if $($1.flex.vfs.$3.embed),cp $$(FLASCC)/usr/share/LSOBackingStore.as $$($1.flex.vfs.output-dir)/LSOBackingStore.as)
	$(if $($1.flex.vfs.$3.http),cp $$(filter %/HTTPBackingStore.as,$$(r.$1.flex.as.sources)) $$($1.flex.vfs.output-dir)/HTTPBackingStore.as)
	$$(flex.as3-comiler) \
		$$(foreach e,builtin playerglobal BinaryData ISpecialFile IBackingStore IVFS InMemoryBackingStore PlayerKernel AlcVFSZip, \
			 -import $$(call flex.regular-path,$$(FLASCC)/usr/lib/$$e.abc)) \
		$(if $($1.flex.vfs.$3.embed),$$(call flex.regular-path,$$($1.flex.vfs.output-dir)/LSOBackingStore.as)) \
		$(if $($1.flex.vfs.$3.http),$$(call flex.regular-path,$$($1.flex.vfs.output-dir)/HTTPBackingStore.as)) \
		$(if $($1.flex.vfs.$3.embed),$$(call flex.regular-path,$$($1.flex.vfs.output-dir)/vfs_$3$3.as)) \
		-outdir $$(call flex.regular-path,$$(dir $$@)) -out $$(notdir $$(basename $$@))

endef

define do-product-def-rule-flex-as

$(eval $1.flex.vfs.output-dir:=$(CPDE_OUTPUT_ROOT)/$($2.output)/obj/$(subst $(CPDE_ROOT)/,,$(r.$1.base)))

$(if $(filter progn,$($1.type)),$(foreach v,$($1.flex.vfs),$(call do-product-def-rule-flex-as-def-vfs,$1,$2,$v)))
$(if $(filter progn,$($1.type)),,$(if $(filter 1, $(r.$1.flex.swig)),$(call do-product-def-rule-flex-as-def-swig,$1,$2,$3)))
$(if $(filter progn,$($1.type)),$(call do-product-def-rule-flex-as-def-progn,$1,$2,$3))

endef

define product-def-rule-flex-as
$(if $(filter flex,$($2.env)),$(call do-product-def-rule-flex-as,$1,$2,$3),)
endef
