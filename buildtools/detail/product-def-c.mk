product-support-types+=lib progn
product-def-c-env-items:= c.flags.cpp c.flags.ld c.sources c.includes c.flags.warning \
                          product.c.includes product.c.flags.ld product.c.defs product.c.ldpathes product.c.libraries \
                          product.c.flags.warning product.c.frameworks product.c.framework-pathes

product-def-all-items+= c.libraries c.frameworks c.framework-pathes c.ldpathes c.linker c.export-symbols \
                        c.flags.lan.all c.flags.lan c.flags.lan.c c.flags.lan.cc c.flags.lan.m c.flags.lan.mm c.lib.type c.env-includes c.env-libraries\
                        product.c.frameworks product.c.framework-pathes product.c.env-includes product.c.env-libraries \
                        c.output-includes product.c.output-includes\
                        $(product-def-c-env-items) $(foreach e,$(dev-env-list),$(addprefix $e.,$(product-def-c-env-items)))

c-source-dir-to-binary-dir=$(foreach f,$1,$(if $(filter $(CPDE_OUTPUT_ROOT)%,$f),$f,$(subst $(CPDE_ROOT),$(CPDE_OUTPUT_ROOT)/$($2.output)/obj,$f)))

c-source-to-object = $(call c-source-dir-to-binary-dir,\
						$(subst .mm,.o,$(filter %.mm,$1))\
						$(subst .m,.o,$(filter %.m,$1))\
						$(subst .cpp,.o,$(filter %.cpp,$1))\
						$(subst .cc,.o,$(filter %.cc,$1))\
						$(subst .c,.o,$(filter %.c,$1))\
						,$2)

# $(call c-generate-env-arg-name-list,product-name,arg-name)
c-generate-env-arg-name-list=$2 $($1.env).$2 $1.$2

c-generate-env-arg-lib-name-list=$1.$2 $2 $($1.env).$2 

c-generate-depend-ld-flags=$(addprefix -L$(CPDE_OUTPUT_ROOT)/,\
                              $(foreach ei,\
								 $(call merge-list, $(r.$1.c.ldpathes) \
                                                  , $(call product-gen-depend-value-list,$1,$($2.env),\
                                                           $(call c-generate-env-arg-name-list,$2,product.c.ldpathes)) \
                                 ),\
                              $(patsubst domain/%,$2/%,$(subst /domain/,/$2/,$(patsubst env/%,$($2.env)/%,$(subst /env/,/$($2.env)/,$(ei)))))) \
                           ) \
                           $(if $(filter cygwin,$(OS_NAME)),-Xlinker --start-group) \
                           $(addprefix -l,$(call merge-list, \
                                                 $(r.$1.c.libraries) $(r.$1.product.c.libraries),\
                                                 $(call product-gen-depend-value-list,$1,$($2.env),\
                                                        $(call c-generate-env-arg-lib-name-list,$2,product.c.libraries)) \
                                            )) \
                           $(addprefix -l,$(foreach lib,$(call merge-list, \
                                                               $(r.$1.c.env-libraries) $(r.$1.product.c.env-libraries),\
                                                               $(call product-gen-depend-value-list,$1,$($2.env),\
                                                                      $(call c-generate-env-arg-name-list,$2,product.c.env-libraries)))\
                                                        , $($($2.env).lib.$(lib)))) \
                            $(addprefix -framework ,\
								 $(sort \
                                    $(r.$1.c.frameworks) \
                                    $(r.$1.$($2.env).c.frameworks) \
                                    $(r.$1.product.c.frameworks) \
                                    $(r.$1.$($2.env).product.c.frameworks) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.frameworks) \
                                    $(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.frameworks) \
                                 )) \
                            $(addprefix -F ,\
								 $(sort \
                                    $(r.$1.c.framework-pathes) \
                                    $(r.$1.$($2.env).c.framework-pathes) \
                                    $(r.$1.product.c.framework-pathes) \
                                    $(r.$1.$($2.env).product.c.framework-pathes) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.framework-pathes) \
                                    $(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.framework-pathes) \
                                 )) \
                           $(if $(filter cygwin,$(OS_NAME)),-Xlinker --end-group) \
                           $(call revert-list,$(call product-gen-depend-value-list,$1,$($2.env), \
                                        $(call c-generate-env-arg-name-list,$2,product.c.flags.ld))) \
                           $(if $(filter 1,$(GCOV)), -fprofile-arcs -ftest-coverage ) \
                           $(if $(filter 1,$(MUDFLAP)), -fmudflap -lmudflap ) \
                           $(r.$1.c.flags.ld) $(r.$1.$($2.env).c.flags.ld) $(r.$1.$2.c.flags.ld) $(r.$1.$($2.env).product.c.flags.ld)

# $(call c-generate-depend-cpp-flags,product-name,domain)
c-generate-depend-cpp-flags=$(foreach i,\
								 $(sort $(r.$1.c.includes) $(r.$1.$($2.env).c.includes) $(r.$1.$2.c.includes) \
                                    $(r.$1.product.c.includes) $(r.$1.$($2.env).product.c.includes) $(r.$1.$2.product.c.includes) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.includes)\
									$(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.includes)\
									$(call product-gen-depend-value-list,$1,$($2.env),$2.product.c.includes)\
                                 ), \
                                -I'$(call $($2.env).regular-path,$(CPDE_ROOT))/$i') \
                            $(foreach ei,\
								 $(sort $(r.$1.c.env-includes) $(r.$1.product.c.env-includes) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.env-includes)), \
                                 $(if $(findstring env,$(subst /, ,$(ei)))\
                                    ,-I'$(call $($2.env).regular-path,$(CPDE_ROOT))/$(subst /env/,/$($($2.env).env-inc)/,$(ei))' \
                                    ,-I'$(call $($2.env).regular-path,$(CPDE_ROOT))/$(ei)/$($2.env)')) \
                            $(foreach oi,\
								 $(call product-gen-depend-list,$($2.env),$1), \
                                 $(foreach d,$(r.$(oi).product.c.output-includes),-I'$(call $($2.env).regular-path,$(call c-source-dir-to-binary-dir,$(r.$(oi).base),$2))/$d')) \
                            $(foreach i,$(r.$1.c.output-includes) $(r.$1.product.c.output-includes), \
                                 -I'$(call $($2.env).regular-path,$(call c-source-dir-to-binary-dir,$(r.$1.base),$2))/$i') \
                            $(addprefix -framework ,\
								 $(sort \
                                    $(r.$1.c.frameworks) \
                                    $(r.$1.$($2.env).c.frameworks) \
                                    $(r.$1.product.c.frameworks) \
                                    $(r.$1.$($2.env).product.c.frameworks) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.frameworks) \
                                    $(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.frameworks) \
                                 )) \
                            $(addprefix -F ,\
								 $(sort \
                                    $(r.$1.c.framework-pathes) \
                                    $(r.$1.$($2.env).c.framework-pathes) \
                                    $(r.$1.product.c.framework-pathes) \
                                    $(r.$1.$($2.env).product.c.framework-pathes) \
									$(call product-gen-depend-value-list,$1,$($2.env),product.c.framework-pathes) \
                                    $(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.framework-pathes) \
                                 )) \
                           $(addprefix -D,$($1.$($2.env).product.c.defs)) \
                           $(addprefix -D,$(r.$1.$2.product.c.defs)) \
                           $(addprefix -D,$(sort $(call product-gen-depend-value-list,$1,$($2.env),product.c.defs))) \
                           $(addprefix -D,$(sort $(call product-gen-depend-value-list,$1,$($2.env),$($2.env).product.c.defs))) \
                           $(addprefix -D,$(sort $(call product-gen-depend-value-list,$1,$($2.env),$2.product.c.defs))) \
                           $(addprefix -D,$(sort $(call product-gen-depend-value-list,$($2.env),$1,$2.product.c.defs))) \
                           $(if $(filter 1,$(GCOV)), -fprofile-arcs -ftest-coverage ) \
                           $(if $(filter 1,$(MUDFLAP)), -fmudflap -fmudflapth ) \
                           $(if $(filter 1,$(GPROF)), -pg ) \
                           $(r.$1.c.flags.cpp) $(r.$1.$($2.env).c.flags.cpp) $(r.$1.$2.c.flags.cpp)

product-def-c-linker-c=$(LINK.c)
product-def-c-linker-cpp=$(LINK.cc)
product-def-c-linker-obj-c=$(LINK.c)
product-def-c-linker-obj-cpp=$(LINK.cc)

product-def-gen-dep-cmd=$(if $(compiler.$(call compiler-category,$($($1.env).CC)).flags.gen-dep),$(compiler.$(call compiler-category,$($($1.env).CC)).flags.gen-dep) $2)

product-def-rule-c-compile-cmd.c=$($($2.env).CC) \
                                 $(compiler.$(call compiler-category,$($($2.env).CC)).flags.warning) \
                                 $($($2.env).CFLAGS) \
                                 $(r.$1.$($2.env).c.flags.warning) \
                                 $(r.$1.c.flags.warning) \
                                 $(sort $(call product-gen-depend-value-list,$1,$($2.env),product.c.flags.warning)) \
                                 $(r.$1.product.c.flags.warning) \
                                 $(r.$1.c.flags.lan.all) \
                                 $(r.$1.$2.c.flags.lan.all) \
                                 $(r.$1.c.flags.lan.c) \
                                 $(r.$1.$2.c.flags.lan.c) \
                                 $($($2.env).CPPFLAGS) \
                                 $($($2.env).TARGET_ARCH) -c

product-def-rule-c-compile-cmd.cc=$($($2.env).CXX) \
                                  $(compiler.$(call compiler-category,$($($2.env).CXX)).flags.warning) \
                                  $($($2.env).CXXFLAGS) \
                                  $(r.$1.$($2.env).c.flags.warning) \
                                  $(r.$1.c.flags.warning) \
                                  $(sort $(call product-gen-depend-value-list,$1,$($2.env),product.c.flags.warning)) \
                                  $(r.$1.product.c.flags.warning) \
                                  $(r.$1.c.flags.lan.all) \
                                  $(r.$1.$2.c.flags.lan.all) \
                                  $(r.$1.c.flags.lan.cc) \
                                  $(r.$1.$2.c.flags.lan.cc) \
                                  $($($2.env).CPPFLAGS) \
                                  $($($2.env).TARGET_ARCH) -c 

product-def-rule-c-compile-cmd.mm=$($($2.env).CXX) $($($2.env).MMFLAGS) \
                                  $(compiler.$(call compiler-category,$($($2.env).CXX)).flags.warning) \
                                  $(r.$1.$($2.env).c.flags.warning) \
                                  $(r.$1.c.flags.warning) \
                                  $(sort $(call product-gen-depend-value-list,$1,$($2.env),product.c.flags.warning)) \
                                  $(r.$1.product.c.flags.warning) \
                                  $(r.$1.c.flags.lan.all) \
                                  $(r.$1.$2.c.flags.lan.all) \
                                  $(r.$1.c.flags.lan.mm) \
                                  $(r.$1.$2.c.flags.lan.mm) \
                                  $($($2.env).CPPFLAGS) \
                                  $($($2.env).TARGET_ARCH) -c 

product-def-rule-c-compile-cmd.m=$($($2.env).CC) $($($2.env).MFLAGS) \
                                 $(compiler.$(call compiler-category,$($($2.env).CC)).flags.warning) \
                                 $(r.$1.$($2.env).c.flags.warning) \
                                 $(r.$1.c.flags.warning) \
                                 $(sort $(call product-gen-depend-value-list,$1,$($2.env),product.c.flags.warning)) \
                                 $(r.$1.product.c.flags.warning) \
                                 $(r.$1.c.flags.lan.all) \
                                 $(r.$1.$2.c.flags.lan.all) \
                                 $(r.$1.c.flags.lan.m) \
                                 $(r.$1.$2.c.flags.lan.m) \
                                 $($($2.env).CFLAGS) \
                                 $($($2.env).CPPFLAGS) \
                                 $($($2.env).TARGET_ARCH) -c

product-def-rule-c-compile-cmd.cpp=$(call product-def-rule-c-compile-cmd.cc,$1,$2)

# $(call compile-rule, binary-file, source-files, product-name,domain)
define product-def-rule-c-compile-rule
$1: $2
	$$(call with_message,$(strip $4).$(strip $3) <== compiling $(patsubst $(r.$3.base)/%,%,$2))\
          $(CPDE_C_COMPILE_PREFIX) $$(call product-def-rule-c-compile-cmd$(suffix $2),$3,$4) $$(call c-generate-depend-cpp-flags,$3,$4) \
          $$(call product-def-gen-dep-cmd,$4,'$$(call $($4.env).regular-path,$$(patsubst %.o,%.d,$$@))') \
          -o '$$(call $($4.env).regular-path,$$@)' '$$(call $($4.env).regular-path,$$<)'

endef

define product-def-c-select-linker
$(strip \
    $(if $(filter .cc,$1),cpp \
	    , $(if $(filter .cpp,$1),cpp \
	        , $(if $(filter .mm,$1),obj-cpp \
	            , $(if $(filter .m,$1),obj-c \
                    , c)))))
endef

# $(call product-def-rule-c-product-for-lib,product-name,domain-name)
define product-def-rule-c-product-for-lib
$(eval r.$1.$2.c.lib.type=$(if $(r.$1.c.lib.type),$(r.$1.c.lib.type),$(if $($2.default-lib-type),$($2.default-lib-type),$($($2.env).default-lib-type))))
$(eval r.$1.$2.product.c.libraries:=$1 $(r.$1.$2.product.c.libraries))
$(eval r.$1.$2.product.c.ldpathes+=$($2.output)/$(if $(r.$1.buildfor),$(r.$1.buildfor)-lib,lib))
$(eval r.$1.$2.product:=$($2.output)/$(if $(r.$1.product),$(r.$1.product),$(r.$1.$2.output)/$(call $($2.env).$(if $(filter static,$(r.$1.$2.c.lib.type)),make-static-lib-name,make-dynamic-lib-name),$1)))
endef

# $(call product-def-rule-c-product-for-progn,product-name,domain-name)
define product-def-rule-c-product-for-progn
$(eval r.$1.$2.product:=$($2.output)/$(if $(r.$1.product),$(r.$1.product),$(r.$1.$2.output)/$(call $($2.env).make-executable-name,$1)))
endef

product-def-rule-c-link-cmd-progn=$$($($4.env).linker.$(r.$1.c.linker)) $2 -o $3 \
                                  $$(call c-generate-depend-ld-flags,$1,$4) \
                                  $$(if $$(r.$1.$4.symbols),$$(foreach e,$$(shell cat $$(r.$1.$4.symbols)),$$(call $($4.env).export-symbols,$$e))) \
                                  $$($($4.env).LDFLAGS) \
                                  $$($($4.env).TARGET_ARCH)
product-def-rule-c-link-cmd-lib-dynamic=$$($($4.env).linker.$(r.$1.c.linker)) $($($4.env).LDFLAGS.share) $2 -o $3 $$(call c-generate-depend-ld-flags,$1,$4) $$($($4.env).TARGET_ARCH)
product-def-rule-c-link-cmd-lib-static=$$($($4.env).AR) $$(ARFLAGS) $3 $2

# $(call product-def-rule-c-symbols,product-name,domain-name)
define product-def-rule-c-symbols
r.$1.$2.symbols:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$1.symbols,$2)

$$(CPDE_OUTPUT_ROOT)/$$(r.$1.$2.product): $$(r.$1.$2.symbols)

$$(r.$1.$2.symbols): $$(r.$1.c.export-symbols) $$(call product-gen-depend-value-list,$1,$2.env,c.export-symbols)
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-symbols.pl \
    --output $$@ \
    $$(addprefix --input ,$$(r.$1.c.export-symbols) $$(call product-gen-depend-value-list,$1,$2.env,c.export-symbols)) \
    --compiler=$(call compiler-category,$($($2.env).CC))

endef

# $(call product-def-rule-c-product,product-name,type,domain-name)
define product-def-rule-c-product

$(eval r.$1.$3.output:=$(if $(r.$1.output),$(r.$1.output),\
                         $(if $(filter lib,$2),\
                              $(if $(r.$1.buildfor),$(r.$1.buildfor)-lib,lib),\
                              $(if $(r.$1.buildfor),$(r.$1.buildfor)-bin,bin))))

$(eval $(if $(filter lib,$2), $(call product-def-rule-c-product-for-lib,$1,$3) \
	      , $(if $(filter progn,$2),$(call product-def-rule-c-product-for-progn,$1,$3)\
                 , $(warning unknown c-product-type of $1: $2)))) \

$(eval r.$1.cleanup+=$(call c-source-to-object,$(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources),$3) \
                     $(patsubst %.o,%.d,$(call c-source-to-object,$(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources),$3)) \
                     $(CPDE_OUTPUT_ROOT)/$(r.$1.$3.product) \
)

$(eval r.$1.c.linker:=$(if $($1.c.linker),$($1.c.linker),$(call product-def-c-select-linker, $(suffix $(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources)))))

auto-build-dirs += $(call c-source-dir-to-binary-dir,$(sort $(dir $(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources))) $(r.$1.base),$3)
auto-build-dirs += $(dir $(CPDE_OUTPUT_ROOT)/$(r.$1.$3.product))

$3.$1: $(CPDE_OUTPUT_ROOT)/$(r.$1.$3.product)

$(if $(filter progn,$2),$(call product-def-rule-c-symbols,$1,$3))

$(CPDE_OUTPUT_ROOT)/$(r.$1.$3.product): $(call c-source-to-object,$(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources),$3)
	$$(call with_message,$(strip $3).$(strip $1) <== linking $(notdir $(r.$1.$3.product)) ...) \
        $(call product-def-rule-c-link-cmd-$(if $(filter progn,$2),progn,lib-$(r.$1.$3.c.lib.type)),$1, $$(filter %.o,$$^), $$@,$3)

$(foreach f,$(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources),$(call product-def-rule-c-compile-rule,$(call c-source-to-object,$f,$3),$f,$1,$3))

$(eval r.$1.$3.makefile.include += $(patsubst %.o,%.d,$(call c-source-to-object,$(r.$1.c.sources) $(r.$1.$($3.env).c.sources) $(r.$1.$3.c.sources),$3)))

ifeq ($(filter progn,$2),progn)
$(eval r.$1.$3.run.libraries+=$(if $(r.$1.buildfor),$(CPDE_OUTPUT_ROOT)/$($3.output)/$(r.$1.buildfor)-lib,) $(CPDE_OUTPUT_ROOT)/$($3.output)/lib)
$(eval r.$1.$3.run.cmd:=$(CPDE_OUTPUT_ROOT)/$(r.$1.$3.product))
endif

endef

define product-def-rule-lib
$(call product-def-rule-c-product,$1,lib,$2)
endef

define product-def-rule-progn
$(call product-def-rule-c-product,$1,progn,$2)
endef
