# {{{ 注册到product-def.mk中

product-support-types+=install
product-def-all-items+=install $(foreach e,$(dev-env-list),$(addprefix $e.,install))
# }}}
# {{{ 给拥护使用的辅助函数，用于定义安装的目标
#$(call def-copy-file,src-file,target-file)
def-copy-file=install-def-sep copy-file $1 $2

#$(call def-copy-file,src-file-list,target-dir)
def-copy-file-list=install-def-sep copy-file-list $2 $1

#$(call def-copy-dir,src-dir,target-dir,postfix-list)
def-copy-dir=install-def-sep copy-dir $1 $2 $3

#$(call def-copy-dir-r,src-dir,target-dir,postfix-list)
def-copy-dir-r=install-def-sep copy-dir-r $1 $2 $3

#$(call def-spack-files,src-dir,target-file,items)
def-spack-items=install-def-sep spack-items $1 $2 $3

#$(call def-cvt-file,src-file,target-file,cvt-way,cvt-args)
def-cvt-file=install-def-sep cvt-file $1 $2 replace $3
def-cvt-file-ex=install-def-sep cvt-file $1 $2 $3 $4
# }}}
# {{{ 预定义的转换方法
tools.cvt.replace.cmd=perl -w $$(CPDE_ROOT)/buildtools/tools/cvt-replace.pl \
                          --input $$^ \
                          --output $$@ \
                          $$(addprefix --config $$(CPDE_ROOT)/,$1)
tools.cvt.replace.dep=$(addprefix $$(CPDE_ROOT)/,$1)

# }}}
# {{{ 实现辅助函数

#                              1            2      3      4       5
# $(call install-def-rule-copy,product-name,source,target,domain,target-target-list)
define install-def-rule-copy
auto-build-dirs+=$(dir $3) 

$(if $5,$5: $3)

.PHONY: $1.install $1.$4.install

$1.install: $1.$4.install

$1.$4.install: $3

$3: $2
	$(call with_message,copy $(subst $(CPDE_ROOT)/,,$2) to $(subst $(CPDE_OUTPUT_ROOT)/,,$3) ...)\
         cp $$< $$@

$(eval r.$1.$4.installed-files += $3)
$(eval r.$1.cleanup += $3)

endef

# $(call install-def-rule-cvt,product-name,source,target,cfg-way,cvt-arg,domain,target-target-list)
define install-def-rule-cvt
$(if $(tools.cvt.$(strip $4).cmd),,$(warning cvt way '$(strip $4)' not support))

auto-build-dirs+=$(dir $3)

$(if $7,$7: $3)

$3: $2 $(call tools.cvt.$(strip $4).dep,$5)
	$(call with_message,convert $(subst $(CPDE_ROOT)/,,$2) to $(subst $(CPDE_OUTPUT_ROOT)/,,$3) ...)\
          $(call tools.cvt.$(strip $4).cmd,$5)

$(eval r.$1.$4.installed-files += $3)
$(eval r.$1.cleanup += $3)

endef

# $(call install-def-rule-one-dir-r,product-name,source-dir,target-dir,postfix-list,domain,target-target-list)
define install-def-rule-one-dir-r
auto-build-dirs+=$(CPDE_OUTPUT_ROOT)/$3

$(foreach f,$(if $(wildcard $(CPDE_ROOT)/$2),\
                 $(shell find $(CPDE_ROOT)/$2 -type f \
                              -not "(" -wholename "*/.svn/*" ")" \
                              $(if $4, "(" -name "*.$(word 1,$4)" $(foreach p,$(wordlist 2,$(words $4),$4), -o -name "*.$p") ")" ))),\
    $(call install-def-rule-copy,$1,$f,$(subst $(CPDE_ROOT)/$2,$(CPDE_OUTPUT_ROOT)/$3,$f),$5,$6) \
)
endef

# $(call install-def-rule-one-dir,product-name,source-dir,target-dir,postfix-list,domain,target-target-list)
define install-def-rule-one-dir
auto-build-dirs+=$(CPDE_OUTPUT_ROOT)/$3

$(if $4,\
     $(foreach postfix,$4 \
          , $(foreach f,$(wildcard $(CPDE_ROOT)/$2/*.$(postfix)) \
               , $(call install-def-rule-copy,$1,$f,$(subst $(CPDE_ROOT)/$2,$(CPDE_OUTPUT_ROOT)/$3,$f),$5,$6))) \
     , $(foreach f,$(if $(wildcard $(CPDE_ROOT)/$2),$(shell find $(CPDE_ROOT)/$2 -maxdepth 1 -type f)) \
           , $(call install-def-rule-copy,$1,$f,$(subst $(CPDE_ROOT)/$2,$(CPDE_OUTPUT_ROOT)/$3,$f),$5,$6)) \
)

endef

#                                         1            2        3           4     5      6
# $(call install-def-rule-one-spack-items,product-name,root-dir,target-file,items,domain,target-target-list)
define install-def-rule-one-spack-items
auto-build-dirs+=$(dir $(CPDE_OUTPUT_ROOT)/$3)

$(if $6,$6: $(CPDE_OUTPUT_ROOT)/$3)

.PHONY: $1.install $1.$5.install

$1.install: $1.$5.install

$1.$5.install: $(CPDE_OUTPUT_ROOT)/$3

$(CPDE_OUTPUT_ROOT)/$3: $$(CPE_SPACK_TOOL) $(addprefix $2/,$4)
	$(call with_message,spack $(notdir $3) ...)$$(CPE_SPACK_TOOL) pack --root=$2 --output=$$@ $(addprefix --input=,$4)

$(eval r.$1.$5.installed-files += $(CPDE_OUTPUT_ROOT)/$3)
$(eval r.$1.cleanup += $(CPDE_OUTPUT_ROOT)/$3)

endef

# }}}
# {{{ 各种不同类型的安装函数入口,#$1是项目名，$2是domain,$3后续参数各自定义

define product-def-rule-install-copy-file
$(call install-def-rule-copy,$1,$(CPDE_ROOT)/$(subst /env/,/$($2.env)/,$(patsubst %/env,%/$($2.env),$(word 1,$3))),$(CPDE_OUTPUT_ROOT)/$($2.output)/$(word 2,$3),$2,$2.$1)
endef

define product-def-rule-install-copy-file-list
$(foreach f,$(wordlist 2,$(words $3), $3),$(call install-def-rule-copy,$1,$(CPDE_ROOT)/$(subst /env/,/$($2.env)/,$(patsubst %/env,%/$($2.env),$f)),$(CPDE_OUTPUT_ROOT)/$($2.output)$(word 1,$3)/$f,$2,$2,$1))
endef

define product-def-rule-install-copy-dir
$(call install-def-rule-one-dir,$1,$(subst /env/,/$($2.env)/,$(patsubst %/env,%/$($2.env),$(word 1,$3))),$($2.output)/$(word 2,$3),$(wordlist 3,$(words $3), $3),$2,$2.$1)
endef

define product-def-rule-install-copy-dir-r
$(call install-def-rule-one-dir-r,$1,$(subst /env/,/$($2.env)/,$(patsubst %/env,%/$($2.env),$(word 1,$3))),$($2.output)/$(word 2,$3),$(wordlist 3,$(words $3), $3),$2,$2.$1)
endef

define product-def-rule-install-spack-items
$(call install-def-rule-one-spack-items,$1,$(word 1,$3),$($2.output)/$(word 2,$3),$(wordlist 3,$(words $3), $3),$2,$2.$1)
endef

define product-def-rule-install-cvt-file
$(if $(word 3,$3),,$(warning convert input file not set))

$(call install-def-rule-cvt,$1,$(CPDE_ROOT)/$(word 1,$3),$(CPDE_OUTPUT_ROOT)/$($2.output)/$(word 2,$3),$(word 3,$3),$(wordlist 4,$(words $3),$3),$2,$2.$1)
endef

# }}}
# {{{ 总入口函数
define product-def-rule-install

$(eval product-def-rule-install-tmp-name:=)
$(eval product-def-rule-install-tmp-args:=)

$(foreach w,$(r.$1.install) $(r.$1.$($2.env).install), \
    $(if $(filter install-def-sep,$w) \
        , $(if $(product-def-rule-install-tmp-name) \
              , $(call product-def-rule-install-$(product-def-rule-install-tmp-name),$1,$2,$(product-def-rule-install-tmp-args)) \
                $(eval product-def-rule-install-tmp-name:=) \
                $(eval product-def-rule-install-tmp-args:=)) \
        , $(if $(product-def-rule-install-tmp-name) \
              , $(eval product-def-rule-install-tmp-args+=$w) \
              , $(eval product-def-rule-install-tmp-name:=$w))))

$(if $(product-def-rule-install-tmp-name) \
    , $(call product-def-rule-install-$(product-def-rule-install-tmp-name),$1,$2,$(product-def-rule-install-tmp-args)))

endef
# }}}
# {{{ 使用示例
#$(product).install:= $(call def-copy-file,source-file, target-file) \
#                     $(call def-copy-file-list,source-file-list, target-dir) \
#                     $(call def-copy-dir-r,source-dir, target-dir, postfix-list) \
#                     $(call def-copy-dir,source-dir, target-dir, postfix-list) \
#                     $(call def-cvt-file,source-file, target-file)
# }}}
