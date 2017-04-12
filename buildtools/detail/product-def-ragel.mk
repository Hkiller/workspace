# {{{ 注册到product-def.mk中
product-support-types+=ragel
product-def-all-items+=ragel
TOOLS_RAGEL=ragel
# }}}
# {{{ 给拥护使用的辅助函数，用于定义安装的目标
#$(call def-ragel-to-c,src,args)
def-ragel-to-c=ragel-def-sep to-c $1 $2
# }}}
# {{{ 辅助实现

#$(call ragel-to-c,project,domain,src,target)
define ragel-to-c
$(eval r.$1.$2.c.sources+=$4)
$(eval r.$1.cleanup += $4)

$4: $3
	$$(call with_message,ragel $$(notdir $$<) to $$(notdir $$@)) \
        $$(TOOLS_RAGEL) -s -G2 $$< -o $$@
endef

# }}}
# {{{ 转换为c实现文件,#$1是项目名，$2是domain,$3后续参数各自定义
define product-def-rule-ragel-to-c
$(call ragel-to-c,$1,$2,$(word 1,$3),$(addprefix $(CPDE_OUTPUT_ROOT)/$($2.output)/obj,$(subst $(CPDE_ROOT),,$(patsubst %.rl,%.c,$(word 1,$3)))))
endef
# }}}
# {{{ 总入口函数
define product-def-rule-ragel

$(eval product-def-rule-ragel-tmp-name:=)
$(eval product-def-rule-ragel-tmp-args:=)

$(foreach w,$(r.$1.ragel) $(r.$1.$2.ragel), \
    $(if $(filter ragel-def-sep,$w) \
        , $(if $(product-def-rule-ragel-tmp-name) \
              , $(call product-def-rule-ragel-$(product-def-rule-ragel-tmp-name),$1,$2,$(product-def-rule-ragel-tmp-args)) \
                $(eval product-def-rule-ragel-tmp-name:=) \
                $(eval product-def-rule-ragel-tmp-args:=)) \
        , $(if $(product-def-rule-ragel-tmp-name) \
              , $(eval product-def-rule-ragel-tmp-args+=$w) \
              , $(eval product-def-rule-ragel-tmp-name:=$w))))

$(if $(product-def-rule-ragel-tmp-name) \
    , $(call product-def-rule-ragel-$(product-def-rule-ragel-tmp-name),$1,$2,$(product-def-rule-ragel-tmp-args)))

endef
# }}}
# {{{ 使用示例
#$(product).ragel:= $(call def-ragel-to-c,a.rl) 
# }}}