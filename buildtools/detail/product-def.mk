CPDE_OUTPUT_ROOT?=$(CPDE_ROOT)/build

ifeq ($(GCOV),1)
CPDE_OUTPUT_ROOT:=$(CPDE_OUTPUT_ROOT)-gcov
endif

ifeq ($(GPROF),1)
CPDE_OUTPUT_ROOT:=$(CPDE_OUTPUT_ROOT)-gprof
endif

ifeq ($(MUDFLAP),1)
CPDE_OUTPUT_ROOT:=$(CPDE_OUTPUT_ROOT)-mudflap
endif

project_repository:=
product-support-types:=
product-def-all-items:=type buildfor depends output version package $(foreach env,$(dev-env-list),$(env).depends)
product-def-not-null-items:=type

product-base = $(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST))))

#$(call product-gen-depend-list-expand,env,cur-list,product-list)
product-gen-depend-list-expand=\
  $(if $(strip $3) \
       , $(call product-gen-depend-list-expand\
                   , $1 \
                   , $(call product-gen-depend-list-expand\
                            , $1 \
                            , $(filter-out $(strip $(firstword $3)),$2) $(strip $(firstword $3)) \
                            , $(r.$(strip $(firstword $3)).depends) $(r.$(strip $(firstword $3)).$(strip $1).depends)) \
                   , $(wordlist 2,$(words $3),$3)) \
       , $2)

#$(call product-gen-depend-list,env,product-list)
product-gen-depend-list=$(call regular-list,$(call product-gen-depend-list-expand,$1,,$(foreach p,$2,$(r.$p.depends) $(r.$p.$(strip $1).depends))))

#$(call product-gen-depend-value-list,product-name,env,value-name-list)
product-gen-depend-value-list=$(call merge-list,,$(foreach p,$(call product-gen-depend-list,$2,$1),$(foreach v,$3,$(r.$p.$v))))

#$(call product-gen-depend-value-list,product-name,env,value-name-list)
product-gen-depend-arg-list=$(call merge-list,,$(foreach p,$(call product-gen-depend-list,$2,$1),$(foreach v,$3,$$(r.$p.$v))))

#$(call product-gen-depend-value-list,product-name,env,)
product-gen-depend-list-with-type=$(foreach p,$(call product-gen-depend-list,$2,$1),$(if $(filter $3,$($p.type)),$p))

#$(call product-gen-depend-value-list,product-name,env,type,value-name-list)
product-gen-depend-arg-list-with-type=$(call merge-list,,$(foreach p,$(call product-gen-depend-list,$2,$1),$(if $(filter $3,$($p.type)),$(foreach v,$4,$$(r.$p.$v)))))

#$(call product-gen-depend-value-list,domain,product-name,type)
product-gen-depend-proj-list-with-type=$(call merge-list,,$(foreach p,$(call product-gen-depend-list,$($2.env),$2),$(if $(filter $3,$($p.type)),$1.$p)))

# $(call product-def-for-domain,product-name,domain)
define product-def-for-domain

$(call assert-not-null,$2.output)

$(eval $2.product-list+=$1)

#copy variables
$(foreach cn,$(product-def-all-items),\
    $(if $($1.$2.$(cn)),$(eval r.$1.$2.$(cn):=$($1.$2.$(cn)))))


.PHONY:$2.$1
$1: $2.$1

$(if $(r.$1.depends),$2.$1: $(addprefix $2.,$(r.$1.depends) $(r.$1.$($2.env).depends)))

$(foreach type,$(if $($1.$2.type),$($1.$2.type),$($1.type)), $(if $(filter $(type),$($2.ignore-types)),,$(call product-def-rule-$(type),$1,$2)))

endef

# $(call product-def,product-name,domain)
define product-def-one
endef

define product-def

$(foreach d,$2,$(if $d,$(eval $d.env?=$(OS_NAME))))

#verify must set variables
$(foreach cn,$(product-def-not-null-items),\
  $(if $($1.$(cn)),,$(error $(cn) of $1 not defined)))

#verify type support
$(foreach type,$($1.type),\
    $(if $(filter $(type),$(product-support-types)),,\
        $(error $1 use not support type $(type), supported types: $(product-support-types))))

#copy variables
$(foreach cn,$(product-def-all-items),\
    $(eval r.$1.$(cn):=$($1.$(cn))))

$(eval $(call build-regular-path,r.$1.base,$(product-base)))

#add product to repository
project_repository+=$1

.PHONY: $1 $1.clean

$(foreach d,$2,$(if $(filter $d,$(using-domain-list)),$(call product-def-for-domain,$1,$d)))

$1.clean:
	$(call with_message,cleaning...)$(RM) $$(r.$1.cleanup)

endef
