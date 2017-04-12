# {{{ 注册到product-def.mk

product-support-types+=meta

# }}}
# {{{ 定义meta转换

# $(call def-meta-convert,product,excell,meta,output)

define def-meta-convert

$1.meta.$(basename $4).input-file:=$(word 1, $(subst ., ,$2)).xls
$1.meta.$(basename $4).input-sheet:=$(word 2, $(subst ., ,$2))
$1.meta.$(basename $4).output:=$4
$1.meta.$(basename $4).meta-lib:=$(word 1, $(subst ., ,$3)).xml
$1.meta.$(basename $4).meta-name:=$(word 2, $(subst ., ,$3))

$(eval $1.meta.list+=$(basename $4))

endef

# }}}
# {{{ 实现：meta导出操作

#$(call product-def-rule-meta,product,domain,module)
define product-def-rule-meta-convert

.PHONY: $(if $(filter 1,$($1.meta.$2.force)),$$($1.meta.$2.output)/$$($1.meta.$3.output))
.DELETE_ON_ERROR: $$($1.meta.output)/$$($1.meta.$2.output)

.DELETE_ON_ERROR: $$($1.meta.$2.output)/$$($1.meta.$3.output)

$1.$2: $$($1.meta.$2.output)/$$($1.meta.$3.output)

$$($1.meta.$2.output)/$$($1.meta.$3.output): $$($1.meta.input)/$$($1.meta.$3.input-file) $$($1.meta.config)/$$($1.meta.$3.meta-lib)
	$(call with_message,$2.$1 generate meta $3)$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-meta.pl \
        --input-file $$^ \
        --input-sheet $$($1.meta.$3.input-sheet) \
        --output $$@ \
        --meta-lib=$$($1.meta.config)/$$($1.meta.$3.meta-lib) \
        --meta-name=$$($1.meta.$3.meta-name) \
        --start-line=$$($1.meta.start-line) \
		$(addprefix --chanel=,$($1.meta.$2.chanel))

endef

#$(call product-def-rule-meta-db,product,domain,module)
define product-def-rule-meta-db

$1: $($1.meta.$2.db)

$($1.meta.$2.db): $(foreach i,$3,$$($1.meta.input)/$$($1.meta.$i.input-file))
	$(call with_message,$2.$1 genetage meta db)echo "" >  $$@
	$(CPE_SILENCE_TAG)$$(foreach f,$3,echo '- $$($1.meta.$$(basename $$f).meta-name): $$($1.meta.$3.output)/$$($1.meta.$$f.output)' >> $$@ ;)

endef

# }}}
# {{{ 实现：international导出操作

#$(call product-def-rule-meta-international,domain,product)
define product-def-rule-meta-international

$(call assert-not-null,$1.meta.languages)
$(call assert-not-null,$1.meta.international-inputs)
$(call assert-not-null,$1.meta.$2.international-output)

auto-build-dirs+=$($1.meta.$2.international-output)

#.PHONY: $$($1.meta.$2.international-output)/strings_$$(firstword $$($1.meta.languages)).stb

.DELETE_ON_ERROR: $$($1.meta.$2.international-output)/strings_$$(firstword $$($1.meta.languages)).stb

$1: $$($1.meta.$2.international-output)/strings_$$(firstword $$($1.meta.languages)).stb

$$($1.meta.$2.international-output)/strings_$$(firstword $$($1.meta.languages)).stb: \
	$$(foreach f,$$($1.meta.international-inputs),$$($1.meta.input)/$$(firstword $$(subst ., ,$$f)).xls)
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-strings.pl \
        $$(addprefix --input $$($1.meta.input)/, $$($1.meta.international-inputs)) \
        $$(addprefix --language ,$$($1.meta.languages)) \
        --output $$($1.meta.$2.international-output)/strings \
        --start-line=$$($1.meta.start-line)

endef

# }}}
# {{{ 实现：定义所有转换操作

#$(call product-def-rule-meta,product)
define product-def-rule-meta-all

.PHONY: $1 $1.$2

$1: $1.$2

$(call assert-not-null,$1.meta.start-line)
$(call assert-not-null,$1.meta.$2.output)
$(call assert-not-null,$1.meta.input)
$(call assert-not-null,$1.meta.config)

$(eval $1.meta.$2.force:=$(call check-content,$($1.meta.$2.output)/chanel.txt,$(if $($1.meta.$2.chanel),$($1.meta.$2.chanel),empty)))

auto-build-dirs+=$$($1.meta.$2.output)

$(eval $(foreach f,$($1.meta.list),$(call product-def-rule-meta-convert,$1,$2,$f)))

endef

# }}}
# {{{ 实现：总入口

define product-def-rule-meta

$(if $($1.meta.$2.output),$(call product-def-rule-meta-all,$1,$2))
$(if $($1.meta.$2.db),$(eval $(call product-def-rule-meta-db,$1,$2,$($1.meta.list))))
$(if $($1.meta.$2.international-output)$($($1.meta.languages)),$(call product-def-rule-meta-international,$1,$2))

endef

# }}}
