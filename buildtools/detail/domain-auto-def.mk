
define domain-target-def
.PHONY: $1

$1: $($1.product-list)

$1.ut: $(if $(filter 0,$($1.ut)) \
                , \
                , $(foreach product,$($1.product-list) \
                            , $(if $($(product).ut),$1.$($(product).ut).run)))

endef

define domain-auto-def-product
$(foreach p,$(call product-gen-depend-list,$($1.env),$($1.product-list) $($1.addition-product-list)) $($1.addition-product-list), \
	$(if $(filter $p,$($1.product-list)) \
        , \
        , $(eval $(call product-def-for-domain,$p,$1))) \
    )

$(if $(filter 0,$($1.ut)) \
    , \
    , $(foreach p, $(foreach u,$($1.product-list), $($(u).ut)) \
                   $(call product-gen-depend-list,$($1.env),$(foreach u,$($1.product-list), $($(u).ut))) \
        , $(if $(filter $p,$($1.product-list)) \
               , \
               , $(eval $(call product-def-for-domain,$p,$1)))))

endef

domain-target-product-dep-def=\
$(if $(r.$1.depends),$(if $(r.$1.$2.product),\
$(addprefix $(CPDE_OUTPUT_ROOT)/,$(r.$1.$2.product)): $(foreach p,$(r.$1.depends),$(addprefix $(CPDE_OUTPUT_ROOT)/,$(r.$p.$2.product)))))

$(foreach domain,$(sort $(using-domain-list)),\
    $(eval $(call domain-auto-def-product,$(domain))) \
	$(eval $(call domain-target-def,$(domain)))\
)

$(if $(filter-out tools,$(using-domain-list))\
    , \
    , $(foreach p,$(project_repository),\
          $(eval $(call product-def-for-domain,$p,default))) \
      $(eval $(call domain-target-def,default)) \
	)

$(foreach env,$(sort $(foreach domain,$(using-domain-list),$($(domain).env))), \
    $(call $(env).check))

$(foreach domain,$(using-domain-list),\
    $(foreach p,$($(domain).product-list),\
	    $(eval $(call domain-target-product-dep-def,$p,$(domain)))\
))
